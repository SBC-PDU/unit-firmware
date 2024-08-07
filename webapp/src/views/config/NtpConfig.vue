<!--
Copyright 2022-2024 Roman Ondráček <mail@romanondracek.cz>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-->

<template>
	<Head>
		<title>{{ $t('core.config.ntp.title') }}</title>
	</Head>
	<Card>
		<template #title>
			{{ $t('core.config.ntp.title') }}
		</template>
		<v-form v-if='config !== null' ref='form' @submit.prevent='submit'>
			<div
				v-for='(server, index) in config.servers'
				:key='index'
			>
				<v-text-field
					v-model='config.servers[index]'
					:label='$t("core.config.ntp.fields.server")'
					:rules='[
						(v: string|null) => FormValidator.isRequired(v, $t("core.config.ntp.messages.emptyServer")),
					]'
					required
					:prepend-inner-icon='mdiServer'
				>
					<template v-if='display.smAndUp.value' #append-inner>
						<v-btn-group class='pb-3' density='compact'>
							<v-btn
								color='success'
								:disabled='config.servers.length === 2'
								size='small'
								@click='config.servers.push("")'
							>
								<v-icon :icon='mdiPlus' />
							</v-btn>
							<v-btn
								color='red'
								:disabled='config.servers.length === 1'
								size='small'
								@click='config.servers.splice(index, 1)'
							>
								<v-icon :icon='mdiMinus' />
							</v-btn>
						</v-btn-group>
					</template>
				</v-text-field>
			</div>
			<v-select
				v-model='config.timezone'
				:label='$t("core.config.ntp.fields.timezone")'
				:items='Object.values(NtpTimezone)'
				:rules='[
					(v: string|null) => FormValidator.isRequired(v, $t("core.config.ntp.messages.emptyTimezone")),
				]'
				required
				:prepend-inner-icon='mdiMapClock'
			/>
			<v-btn
				color='primary'
				type='submit'
				:prepend-icon='mdiPencil'
			>
				{{ $t('core.actions.edit') }}
			</v-btn>
		</v-form>
	</Card>
</template>

<script lang='ts' setup>
import { mdiPencil, mdiMapClock, mdiServer, mdiMinus, mdiPlus } from '@mdi/js';
import { Head } from '@vueuse/head';
import { onBeforeMount, Ref, ref } from 'vue';
import { useI18n } from 'vue-i18n';
import { toast } from 'vue3-toastify';
import { useDisplay } from 'vuetify';
import { VForm } from 'vuetify/components';

import Card from '@/components/Card.vue';
import FormValidator from '@/helpers/formValidator';
import ConfigService from '@/services/ConfigService';
import { useLoadingSpinnerStore } from '@/store/loadingSpinner';
import { NtpConfig, NtpTimezone } from '@/types/config';

const display = useDisplay();
const i18n = useI18n();
const loadingSpinner = useLoadingSpinnerStore();
const service = new ConfigService();

const config = ref<NtpConfig|null>(null);
const form: Ref<VForm | null> = ref(null);

/**
 * Loads NTP configuration
 */
async function loadData(): Promise<void> {
	loadingSpinner.show();
	try {
		config.value = await service.getNtp();
	} finally {
		loadingSpinner.hide();
	}
}

/**
 * Submits form
 */
async function submit(): Promise<void> {
	if (form.value === null) {
		return;
	}
	const { valid } = await form.value.validate();
	if (!valid || config.value === null) {
		return;
	}
	loadingSpinner.show();
	try {
		await service.setNtp(config.value);
		toast.success(i18n.t('core.config.ntp.messages.success'));
	} catch {
		toast.error(i18n.t('core.config.ntp.messages.error'));
	} finally {
		loadingSpinner.hide();
	}
}

onBeforeMount(async () => await loadData());
</script>
