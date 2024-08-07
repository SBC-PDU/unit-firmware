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
import { type AxiosResponse } from 'axios';

import { ApiClient } from '@/services/ApiClient';
import { type AuthConfig, type Credentials } from '@/types/auth';

/**
 * Authentication service
 */
export default class AuthenticationService extends ApiClient {

	/**
	 * Sets user credentials
	 * @param {AuthConfig} config Authentication configuration
	 */
	public async setCredentials(config: AuthConfig): Promise<void> {
		await this.getClient().put('/auth', config);
	}

	/**
	 * Verifies user credentials
	 * @param {Credentials} credentials User credentials
	 * @return {Promise<AxiosResponse>} Axios response
	 */
	public async verify(credentials: Credentials): Promise<void> {
		await this.getClient().get('/auth', { auth: {
			username: credentials.username,
			password: credentials.password,
		} });
	}

}
