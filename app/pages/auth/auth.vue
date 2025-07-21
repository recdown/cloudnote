<template>
  <view class="container">

    <register v-if="flag"></register>
    <login v-else></login>
    <view class="container_footer">
      <span @click="flag=!flag">{{ flag ? '已有账号? 去登录' : '没有账号? 去注册' }}</span>
    </view>
  </view>
</template>

<script lang="ts" setup>
import {ref} from 'vue';
import register from '@/pages/auth/register.vue'
import login from '@/pages/auth/login.vue'
import useCheckLogin from '@/utils/auth'

const flag = ref(false)

useCheckLogin()
</script>

<style lang="scss" scoped>
.container {

  &_footer {

    text-align: center;
  }

}
</style>
