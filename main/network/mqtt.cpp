#include "network/mqtt.h"

static const char *TAG = "MQTT";

MqttLastWillAndTestament::MqttLastWillAndTestament(const std::string &topic, const std::string &message, const int qos, const bool retain): topic(topic), message(message), qos(qos), retain(retain) {}

const std::string &MqttLastWillAndTestament::getTopic() {
    return this->topic;
}

const std::string &MqttLastWillAndTestament::getMessage() {
    return this->message;
}

int MqttLastWillAndTestament::getQos() const {
    return this->qos;
}

bool MqttLastWillAndTestament::getRetain() const {
    return this->retain;
}

MqttConfig::MqttConfig() {
    memset(&this->config, 0, sizeof(esp_mqtt_client_config_t));
    nvs.getString("uri", this->brokerUri);
    this->config.uri = this->brokerUri.c_str();
    nvs.getString("username", this->username);
    this->config.username = this->username.c_str();
    nvs.getString("password", this->password);
    this->config.password = this->password.c_str();
    //this->config.use_global_ca_store = true;
    this->config.keepalive = 15;
}

const esp_mqtt_client_config_t &MqttConfig::get() {
    return this->config;
}

void MqttConfig::setLastWillAndTestament(MqttLastWillAndTestament *lwt) {
    this->lwt = lwt;
    if (lwt == nullptr) {
        return;
    }
    this->config.lwt_topic = lwt->getTopic().c_str();
    this->config.lwt_msg = lwt->getMessage().c_str();
    this->config.lwt_msg_len = lwt->getMessage().length();
    this->config.lwt_qos = lwt->getQos();
    this->config.lwt_retain = lwt->getRetain();
}

static void inline log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

std::map<std::string, Mqtt::subscribe_callback_t> Mqtt::callbacks = std::map<std::string, Mqtt::subscribe_callback_t>();
esp_mqtt_client_handle_t Mqtt::handle = nullptr;
Mqtt::connect_callback_t Mqtt::onConnect;

Mqtt::Mqtt(const MqttConfig &mqttConfig): config(mqttConfig) {
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
    esp_mqtt_client_config_t config = this->config.get();
    Mqtt::handle = esp_mqtt_client_init(&config);
    if (Mqtt::handle == nullptr) {
        ESP_LOGE(TAG, "Unable to create client handle.");
    }
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(Mqtt::handle, static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID), &Mqtt::eventHandler, reinterpret_cast<void*>(this)));
    (esp_mqtt_client_start(Mqtt::handle));
    if (Mqtt::handle == nullptr) {
        ESP_LOGE(TAG, "MQTT client handle in NULL");
    }
}

void Mqtt::eventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
    Mqtt *mqtt = reinterpret_cast<Mqtt *>(handler_args);

    if (event->client == nullptr) {
        ESP_LOGE(TAG, "MQTT client handle in NULL");
    }

    switch (static_cast<esp_mqtt_event_id_t>(event_id)) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Connected to the MQTT broker.");
            for (std::map<std::string, Mqtt::subscribe_callback_t>::iterator it = Mqtt::callbacks.begin(); it != Mqtt::callbacks.end(); ++it) {
                mqtt->subscribe(it->first, it->second, 1);
            }
            Mqtt::handle = event->client;
            if (Mqtt::onConnect) {
                Mqtt::onConnect(mqtt, base, event);
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Disconnected from the MQTT broker.");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            break;
        case MQTT_EVENT_PUBLISHED:
            break;
        case MQTT_EVENT_DATA: {
            std::string topic(event->topic, event->topic_len);
            ESP_LOGI(TAG, "Received data from topic \"%s\":", topic.c_str());
            ESP_LOG_BUFFER_HEXDUMP(TAG, event->data, event->data_len, ESP_LOG_INFO);
            if (Mqtt::callbacks.find(topic) != Mqtt::callbacks.end()) {
                Mqtt::subscribe_callback_t callback = Mqtt::callbacks[topic];
                callback(event);
            } else {
                ESP_LOGE(TAG, "Unable to find callback for topic: \"%s\".", topic.c_str());
            }
            break;
        }
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            ESP_LOGI(TAG, "Other event id: %d", event->event_id);
            break;
    }
}

int Mqtt::publishString(const std::string &topic, const std::string &data, const int qos, const bool retain) {
    if (Mqtt::handle == nullptr) {
        ESP_LOGE(TAG, "MQTT client handle in NULL");
        return -1;
    }
    int msgId = esp_mqtt_client_publish(Mqtt::handle, topic.c_str(), data.c_str(), data.length(), qos, retain);
    ESP_LOGI(TAG, "Published \"%s\" to the topic \"%s\" with QoS %d. Message ID: %d", data.c_str(), topic.c_str(), qos, msgId);
    if (msgId == -1) {
        ESP_LOGE(TAG, "Failed to publish \"%s\" to the topic \"%s\" with QoS %d.", data.c_str(), topic.c_str(), qos);
    }
    return msgId;
}

void Mqtt::setOnConnect(Mqtt::connect_callback_t onConnect) {
    Mqtt::onConnect = onConnect;
}

int Mqtt::subscribe(const std::string &topic, const Mqtt::subscribe_callback_t &callback, int qos) {
    if (Mqtt::handle == nullptr) {
        ESP_LOGE(TAG, "MQTT client handle in NULL");
        return -1;
    }
    Mqtt::callbacks[topic] = callback;
    int msgId = esp_mqtt_client_subscribe(Mqtt::handle, topic.c_str(), qos);
    ESP_LOGI(TAG, "Subscribed to the topic \"%s\" with QoS %d. Mesasge ID: %d", topic.c_str(), qos, msgId);
    if (msgId == -1) {
        ESP_LOGE(TAG, "Failed to suscribe to the topic \"%s\" with QoS %d.", topic.c_str(), qos);
    }
    return msgId;
}

int Mqtt::unsubscribe(const std::string &topic) {
    if (Mqtt::handle == nullptr) {
        ESP_LOGE(TAG, "MQTT client handle in NULL");
        return -1;
    }
    Mqtt::callbacks.erase(topic);
    int msgId = esp_mqtt_client_unsubscribe(Mqtt::handle, topic.c_str());
    ESP_LOGI(TAG, "Unsubscribed from the topic \"%s\". Mesasge ID: %d", topic.c_str(), msgId);
    if (msgId == -1) {
        ESP_LOGE(TAG, "Failed to unsuscribe from the topic \"%s\".", topic.c_str());
    }
    return msgId;
}

std::string getBaseTopic() {
    return "sbc_pdu/" + Wifi::getPrimaryMacAddress();
}