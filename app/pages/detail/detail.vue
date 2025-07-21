<script lang="ts" setup>
import {onLoad} from '@dcloudio/uni-app'
import {ref} from 'vue';
import {deleteNoteApi, getNoteApi} from '../../api/article';
import dayjs from 'dayjs'


const data = ref({})

const getNote = async (id: number) => {
  const res = await getNoteApi(id)
  data.value = res
}
onLoad((data) => {
  getNote(data.id)

})

const deleteNote = async (id: number) => {
  await deleteNoteApi(id)
  uni.navigateBack()
}

const toEdit = (id: Number) => {
  uni.navigateTo({
    url: `/pages/edit/edit?id=${id}`
  })
}
</script>

<template>
  <view class="detail">
    <view class="detail-title">
      <p>{{ data.data?.title }}</p>
      <span>{{ dayjs(data.data?.createAt.substring(0, 19)).format('YYYY-MM-DD HH:mm:ss') }}</span>

    </view>
    <view class="detail-set">
      <i class="detail-set-edit" @click="toEdit(data.data.id)">编辑</i>
      <i class="detail-set-delete" @click="deleteNote(data.data.id)">删除</i>
    </view>
    <view class="detail-content">{{ data.data?.content }}</view>

  </view>
</template>

<style scoped>

</style>
