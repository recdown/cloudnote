<template>
  <view class="index">
    <view v-for="item in list" :key="item.id" class="index-container" @click="toPage(item.id)">
      <view class="index-container-title">
        {{ item.title }}
      </view>
      <view class="index-container-content">
        <img :src="`http://localhost:3000/${item.cover}`" alt="封面"/>
        <view class="index-container-content-text">
          {{ item.content }}
        </view>

      </view>

    </view>


  </view>
</template>

<script lang="ts" setup>
import {onMounted, ref} from 'vue';
import {onShow} from '@dcloudio/uni-app'
import useCheckLogin from '@/utils/auth'
import {getNoteListApi} from '../../api/article';

useCheckLogin()

const list = ref([])
const getNotelist = async () => {
  const res = await getNoteListApi()
  list.value = res.data
  console.log('res.data', res.data)

}

onMounted(() => {
  getNotelist()
})
onShow(() => {
  getNotelist()
})
const toPage = (id: number) => {
  uni.navigateTo({
    url: `/pages/detail/detail?id=${id}`
  })
}


</script>

<style lang="scss" scoped>

</style>
