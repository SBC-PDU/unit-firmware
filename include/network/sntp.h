/**
 * Copyright 2022-2024 Roman Ondráček <mail@romanondracek.cz>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <cstring>
#include <ctime>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <sys/time.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_sntp.h>

#include "nvsManager.h"
#include "mcp7940n.h"

/**
 * Simple Network Time Protocol client
 */
class Ntp {
	public:
		/**
		 * Construct a new SNTP client instance
		 * @param rtc MCP7940N RTC instance
		 */
		explicit Ntp(Mcp7940n *rtc);

		/**
		 * Obtains the current time from NTP servers
		 */
		static void obtainTime();

		/**
		 * Notifies time syncchronization
		 * @param tv Time
		 */
		static void notifySyncronization(struct timeval *tv);

		/// Timezones
		static std::map<std::string, std::string> timezones;

	private:
		/// Logger tag
		static constexpr const char *TAG = "NTP";
		/// NVS manager
		NvsManager nvs = NvsManager("ntp");
		/// MCP7940N RTC instance
		static Mcp7940n *rtc;
		/// NTP server list
		std::vector<std::string> servers;
		/// Timezone
		std::string timezone;
};
