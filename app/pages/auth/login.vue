<script lang="ts" setup>
import {ref} from 'vue';
import {rules} from '../../utils/rule';
import {LoginDTO} from '../../dto/auth.dto';
import {loginApi} from '../../api/auth';
import sha256 from 'js-sha256'
import useCheckLogin from '@/utils/auth'

useCheckLogin()

const formRef = ref()


const formData = ref<LoginDTO>({
  username: '',
  password: ''
})

const submit = async () => {
  try {
    await formRef.value?.validate()
  } catch {
    uni.showToast({title: '请填写完整信息', icon: 'error'})
    return
  }

  formData.value.password = sha256(formData.value.password)
  // 请求登录接口
  const res = await loginApi(formData.value)
  if (res) {
    uni.setStorageSync('token', res.token)
    uni.showToast({title: '登录成功', icon: 'success'})
    await uni.reLaunch({url: '/pages/index/index'})

  } else {
    uni.showToast({title: '登录失败，请检查用户名密码', icon: 'none'})
  }


}
const handleKeyDown = (event: KeyboardEvent) => {
  if (event.key === 'Enter') {
    submit();
  }
};
</script>

<template>
  <view class='login'>
    <view class="login_container">
      <uni-forms ref="formRef" :model="formData" :rules="rules">

        <uni-forms-item label="用户名" name="username">

          <uni-easyinput v-model='formData.username' placeholder="请输入用户名" @keydown.enter="handleKeyDown"/>

        </uni-forms-item>

        <uni-forms-item label="密码" name="password">
          <uni-easyinput v-model='formData.password' placeholder="请输入密码" type="password"
                         @keydown.enter="handleKeyDown"/>
        </uni-forms-item>

        <button type="primary" @click="submit">登录</button>


      </uni-forms>
    </view>

  </view>
</template>

<style lang="scss" scoped>

</style>
