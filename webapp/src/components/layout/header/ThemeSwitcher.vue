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
	<v-btn
		v-model='themeSwitch'
		:icon='themeSwitch ? mdiWeatherNight : mdiWeatherSunny'
		@click='toggleTheme'
	/>
</template>

<script lang='ts' setup>
import { mdiWeatherNight, mdiWeatherSunny } from '@mdi/js';
import { onBeforeMount, ref, type Ref } from 'vue';
import { useTheme } from 'vuetify';

const theme = useTheme();
const themeSwitch: Ref<boolean> = ref(false);

onBeforeMount((): void => {
	themeSwitch.value = theme.global.name.value === 'dark';
});

/**
 * Toggles the theme
 */
function toggleTheme(): void {
	theme.global.name.value = theme.global.name.value === 'dark' ? 'light' : 'dark';
	themeSwitch.value = theme.global.name.value === 'dark';
}
</script>
