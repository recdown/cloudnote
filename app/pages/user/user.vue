<script lang="ts" setup>
import {onMounted, ref} from 'vue'
import useCheckLogin from '@/utils/auth'
import {getUserApi} from '../../api/user'

useCheckLogin()

const userInfo = ref({})

onMounted(() => {
  getUserApi()
      .then((res) => {
        console.log('getUserApi 请求成功:', res)

        userInfo.value = res

      })
      .catch(err => {
        console.error('getUserApi 请求失败:', err)

      })
})


//退出登录
const logout = () => {
  uni.removeStorageSync('token')
  uni.removeStorageSync('userInfo')
  uni.reLaunch({
    url: '/pages/auth/auth'
  })
}
</script>

<template>
  <view>

    <view class="user">
      <view class="user-header">
        <view class="user-header-left">
          <!-- <image src="/static/uni.png" </image> -->
          <view class="user-header-left-info">
            <text class="user-header-left-info-nickname"><span>昵称:</span>{{ userInfo.nickname }}</text>

          </view>
        </view>


      </view>

      <view class="user-menu">
        <view class="user-menu-list">

          <view class="user-menu-list-item" @click="logout">退出登录<span>></span></view>
        </view>
      </view>
    </view>

  </view>
</template>

<style scoped>

</style>
