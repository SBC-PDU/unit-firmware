#pragma once

#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <functional>
#include <map>
#include <string>

#include <esp_log.h>
#include <mqtt_client.h>

#include "nvs_manager.h"
#include "network/wifi.h"

/**
 * MQTT last will and testament configuration
 */
class MqttLastWillAndTestament {
    public:
        /**
         * Constructor
         * @param topic MQTT topic for publishing the last will and testament
         * @param message Content of the last will and testament
         * @param qos QoS of the last will and testament
         * @param retain Retain flag of the last will and testament
         */
        MqttLastWillAndTestament(const std::string &topic, const std::string &message, const int qos, const bool retain);

        /**
         * Returns the MQTT topic for pusblishing the last will and testament
         * @return MQTT topic for pusblishing the last will and testament
         */
        const std::string &getTopic();

        /**
         * Returns the content of the last will and testament
         * @return Content of the last will and testament
         */ 
        const std::string &getMessage();

        /**
         * Returns the QoS of the last will and testament
         * @return QoS of the last will and testament
         */
        int getQos() const;

        /**
         * Returns the retain flag of the last will and testament
         * @return Rertain flag of the last will and testament
         */
        bool getRetain() const;

    private:
        /// MQTT topic for pusblishing the last will and testament
        std::string topic;
        /// Content of the last will and testament
        std::string message;
        /// QoS of the last will and testament
        int qos;
        /// Retain flag of the last will and testament
        bool retain;
};


/**
 * MQTT client configuration
 */
class MqttConfig {
    public:
        /**
         * Constructor
         */
        MqttConfig();

        /**
         * Returns MQTT client configuration
         * @return MQTT client configuration
         */
        const esp_mqtt_client_config_t &get();

        /**
         * Sets the last will and testament configuration
         * @param lwt MQTT last will and testament configuration
         */
        void setLastWillAndTestament(MqttLastWillAndTestament *lwt);

    private:
         /// MQTT broker URI
        std::string brokerUri;
        /// MQTT client config
        esp_mqtt_client_config_t config;
        /// Last will and testament
        MqttLastWillAndTestament* lwt = nullptr;
        /// NVS manager
        NvsManager nvs = NvsManager("mqtt");
        /// MQTT broker username
        std::string username;
        /// MQTT broker password
        std::string password;
        /// Connection keepalive interval
        uint8_t keepalive;
};

/**
 * MQTT client
 */
class Mqtt {
    public:
        /// Connection callback type definition
        typedef std::function<void(Mqtt* client, esp_event_base_t base, esp_mqtt_event_handle_t event)> connect_callback_t;
        /// Subscribe callback type definition
        typedef std::function<void(esp_mqtt_event_handle_t event)> subscribe_callback_t;

        /**
         * Constructor
         * @param config MQTT client configuration manager
         */
        Mqtt(const MqttConfig &config);

        /**
         * Event handler registered to receive MQTT events
         * This function is called by the MQTT client event loop.
         * @param handler_args user data registered to the event.
         * @param base Event base for the handler(always MQTT Base in this example).
         * @param id The id for the received event.
         * @param data The data for the event, esp_mqtt_event_handle_t.
         */
        static void eventHandler(void *handler_args, esp_event_base_t base, int32_t id, void *data);

        /**
         * Publishes the data into the topic
         * @param topic Topic
         * @param data Data to send
         * @param qos QoS of published message
         * @param retain Retain flag
         * @return Message ID of the published message on success or -1 on failure
         */
        int publishString(const std::string &topic, const std::string &data, const int qos, const bool retain);

        /**
         * Subscibes to the topic
         * @param topic Topic
         * @param callback Callback
         * @param qos QoS
         * @return Message ID of the published message on success or -1 on failure
         */
        int subscribe(const std::string &topic, const Mqtt::subscribe_callback_t &callback, int qos);

        /**
         * Unsubscibes from the topic
         * @param topic Topic
         * @return Message ID of the published message on success or -1 on failure
         */
        int unsubscribe(const std::string &topic);

        /**
         * Sets the connection callback
         * @param onConnect Connection callback
         */
        void setOnConnect(Mqtt::connect_callback_t onConnect);

        /**
         * Returns base MQTT topic
         * @return Base MQTT topic
         */
        std::string getBaseTopic();

    protected:
        /// Subscribe callbacks
        static std::map<std::string, Mqtt::subscribe_callback_t> callbacks;
        /// MQTT client handle
        static esp_mqtt_client_handle_t handle;

    private:
        /// MQTT client configuration manager
        MqttConfig config;
        /// On connect callback
        static Mqtt::connect_callback_t onConnect;
};