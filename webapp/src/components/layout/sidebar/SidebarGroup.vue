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
	<v-list-group
		:prepend-icon='subGroup ? "" : item.icon'
		:subgroup='subGroup'
		:value='item.title'
	>
		<template #activator='{ props }'>
			<v-list-item :title='item.title' density='compact' v-bind='props' />
		</template>
		<div v-for='(navItem, idx) in item.children' :key='idx'>
			<SidebarGroup
				v-if='navItem.children !== undefined && navItem.children.length > 0'
				:item='navItem'
				sub-group
			/>
			<SidebarItem
				v-else
				:item='navItem'
			/>
		</div>
	</v-list-group>
</template>

<script lang='ts' setup>
import { type PropType } from 'vue';

import SidebarItem from '@/components/layout/sidebar/SidebarItem.vue';
import { SidebarLink } from '@/types/sidebar';

defineProps({
	/// Sidebar items to render
	item: {
		type: Object as PropType<SidebarLink>,
		required: true,
	},
	/// Is subgroup?
	subGroup: {
		type: Boolean,
		default: false,
		required: false,
	},
});
</script>
