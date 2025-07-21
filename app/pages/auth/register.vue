<script lang="ts" setup>
import {ref} from 'vue';
import {rules} from '../../utils/rule';
import {RegisterDTO} from '../../dto/auth.dto';
import {registerApi} from '../../api/auth';
import sha256 from 'js-sha256'
import useCheckLogin from '@/utils/auth'

useCheckLogin()
const formRef = ref()


const formData = ref<RegisterDTO>({
  username: '',
  password: '',
  nickname: ''
})

const submit = async () => {

  // try语句块，用于捕获异常
  try {
    // 调用formRef.value.validate()方法，验证表单
    await formRef.value.validate();

    // 将密码进行sha256加密
    formData.value.password = sha256(formData.value.password)

    // 调用registerApi方法，传入formData，注册用户
    await registerApi(formData.value)

    uni.showToast({
      title: '注册成功',
      icon: 'success'
    })
    uni.navigateTo({
      url: '/pages/auth/login'
    })


  } catch (e) {


    uni.showToast({
      title: e.data.message,
      icon: 'error'
    })

  }


}

const handleKeyDown = (event: KeyboardEvent) => {
  if (event.key === 'Enter') {
    submit();
  }
};
</script>

<template>
  <view class='register'>
    <view class="register_container">
      <uni-forms ref="formRef" :model="formData" :rules="rules">

        <uni-forms-item label="用户名" name="username">

          <uni-easyinput v-model='formData.username' placeholder="请输入用户名" @keydown.enter="handleKeyDown"/>

        </uni-forms-item>

        <uni-forms-item label="密码" name="password">
          <uni-easyinput v-model='formData.password' placeholder="请输入密码" type="password"
                         @keydown.enter="handleKeyDown"/>
        </uni-forms-item>
        <uni-forms-item label="昵称" name="nickname">
          <uni-easyinput v-model='formData.nickname' placeholder="请输入昵称"/>
        </uni-forms-item>
        <button type="primary" @click="submit">注册</button>

      </uni-forms>
    </view>

  </view>
</template>

<style lang="scss" scoped>

</style>
