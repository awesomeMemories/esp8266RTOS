/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include <esp_http_server.h> //Implementacion Servidor protocolo http
#include <sys/param.h> //Para usar la funcion MIN()
//------ Peripherals------
#include "driver/gpio.h"  //blink led
#include "freertos/queue.h" //queue

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;

//-------Queue------------
static xQueueHandle gpio_manage_queue = NULL;
bool doorState = false;

//-------Peripherals--------
#define GPIO_OUTPUT_IO_2    12    //port pin where is conecting led
#define GPIO_OUTPUT_PIN_LED  (1ULL<<GPIO_OUTPUT_IO_2) //to set te pin

//Task manage GPIO: to blinker the led
static void gpio_task_blinker(void *arg)
{
    //---Config. Peripherals
    gpio_config_t io_conf; //variable to config the pin
    //BLINKER LED PIN
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_LED;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //------------------------
    int counter = 0;

    while (1) {
        //ESP_LOGI(TAG, "Led cnt: %d\n", counter++);
        ESP_LOGI(TAG, "FSgpio:%u", uxTaskGetStackHighWaterMark(NULL)); //monitore Free stack memory
        vTaskDelay(2000 / portTICK_RATE_MS);//2000msec
	
	//short doorStateOC = counter % 2;
	uint32_t doorStateOC;
        if(xQueueReceive(gpio_manage_queue, &doorStateOC, ( TickType_t ) 10 ))
	{
           ESP_LOGI(TAG, "state:%d,%d\n", doorStateOC, counter++);
        }

	//gpio_num: GPIO number. If you want to set the output level of e.g. GPIO16, gpio_num should be GPIO_NUM_16 (16);
	//level: Output level. 0: low ; 1: high
        gpio_set_level(GPIO_OUTPUT_IO_2, doorStateOC);
    }
}


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

//HTTP SERVER
/* Our URI handler function to be called during GET /uri request */
esp_err_t get_handler(httpd_req_t *req)
{
    //Detect type of request
    
    /* Send a simple response */
    const char resp[] = "GET resp.";
    ESP_LOGI(TAG, "FS server: '%u'", uxTaskGetStackHighWaterMark(NULL)); //monitore Free stack memory
    //Read req value and set 
    uint32_t door;
    if(doorState)
    {
      doorState = false;
      door = 1;
    }
    else
    {
      doorState = true;
      door = 0;
    }
    // Send an uint32_t.  Wait for 10 ticks for space to become
    // available if necessary.
    if( xQueueSendToFront( gpio_manage_queue, ( void * ) &door, ( TickType_t ) 10 ) != pdPASS )
    {
         // Failed to post the message, even after 10 ticks.
         ESP_LOGI(TAG, "Error sent data");
    }

    httpd_resp_send(req, resp,strlen(resp));
    return ESP_OK;
}

/* Our URI handler function to be called during POST /uri request */
esp_err_t post_handler(httpd_req_t *req)
{
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    char content[100];

    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }

    /* Send a simple response */
    const char resp[] = "URI POST Response";
    httpd_resp_send(req, resp,strlen(resp));
    return ESP_OK;
}

/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
    .uri      = "/kk",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

/* URI handler structure for POST /uri */
httpd_uri_t uri_post = {
    .uri      = "/kk",
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};

/* Function for starting the webserver */
httpd_handle_t start_webserver(void)
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

/* Function for stopping the webserver */
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}


void wifi_init_sta(void)
{
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    //Event groups are a synchronization mechanism used for communication between tasks
    //An event group is essentially a collection of individual flags, each representing a specific event or condition.
    s_wifi_event_group = xEventGroupCreate();

    //It initialized the default TCP/IP stack on your ESP device. This included tasks like memory allocation, 
    //setting up network interfaces (Wi-Fi or Ethernet), and starting essential network services.
    tcpip_adapter_init();

    //This function is used to create the default event loop. This event loop provides a mechanism for components and 
    //tasks in your application to communicate and synchronize using events.
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    //esp_wifi_init initiates internal tasks responsible for managing Wi-Fi functionality, such as scanning for
    //networks, handling connections, and data transmission/reception.
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    //esp_event_handler_register is a function used to register a handler function for specific events within your 
    //application. This mechanism allows different parts of your code to communicate and synchronize based on events 
    //being posted to a central event loop.
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS
        },
    };

    /* Setting a password implies station will connect to all security modes including WEP/WPA.
        * However these modes are deprecated and not advisable to be used. Incase your Access point
        * doesn't support WPA2, these mode can be enabled by commenting below line */

    if (strlen((char *)wifi_config.sta.password)) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    //esp_event_handler_unregister is a function used to unregister a previously registered event handler. 
    //This allows you to stop a specific function from being called when corresponding events are posted to the 
    //event loop.
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));

    //vEventGroupDelete is used to delete an event group that was previously created using xEventGroupCreate 
    vEventGroupDelete(s_wifi_event_group);
    
    if (bits & WIFI_CONNECTED_BIT) {
       //Si se logró conectar => se puede levantar un Servidor 
       if(start_webserver())
       {
          ESP_LOGI(TAG, "Server start OK");
       }
       else
       {
          ESP_LOGI(TAG, "ERROR No se pudo crear Servidor");
       }
    } 
}

void app_main()
{
    //-----------------------------------------
    //init Non-Volatile Storage
    //By initializing NVS first, you ensure that it's ready to be used by any tasks you create later that might 
    //need to access or modify this persistent storage.
    //
    //Initializing NVS first ensures a clean startup sequence and avoids potential conflicts.
    ESP_ERROR_CHECK(nvs_flash_init());

    wifi_init_sta();

    //create a queue to handle gpio manage from http server
    gpio_manage_queue = xQueueCreate(10, sizeof(uint32_t));
    if( gpio_manage_queue == 0 )//stop program or only use the task that don't use queue
    {
       ESP_LOGI(TAG, "Queue was not created and must not be used");
    }

    //---Task to manage GPIO: blinker led
    //xTaskCreate(gpio_task_blinker, "gpio_task_blinker", 1048, NULL, 10, NULL);
    xTaskCreate(gpio_task_blinker, "gpio_task_blinker", 1500, NULL, 10, NULL);
    //          |                   |                   |     |     |   |
    //          |                   |                   |     |     |   Used to pass a handler to the created task
    //          |                   |                   |     |     priority
    //          |                   |                   |     A value that is passed as the parameter to the created
    //          |                   |                   |      task.
    //          |                   |                   The number of words(not bytes) 
    //          |                   A descriptive name for the task   
    //          Pointer to the task entry function

    //Note: esp8266RTOS/components/freertos/port/esp8266/include/freertos/FreeRTOSConfig.h
    //#define configMAX_PRIORITIES		15
}
