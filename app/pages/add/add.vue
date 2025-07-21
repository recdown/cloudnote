<script lang="ts" setup>
import {ref} from 'vue';
import useCheckLogin from '@/utils/auth'
import {NoteDTO} from '../../dto/note.dto';
import {addNoteApi} from '../../api/article';

useCheckLogin()

const formRef = ref()
const formData = ref<NoteDTO>({
  title: '',
  content: '',
  cover: ''
})

const rules = {
  title: {
    rules: [{required: true, errorMessage: '笔记标题不能为空'},]
  },
  content: {
    rules: [{required: true, errorMessage: '笔记内容不能为空'},]
  },
  cover: {
    rules: [{required: true, errorMessage: '封面不能为空'},]
  }
}

const submit = async () => {
  try {
    await formRef.value?.validate().then((res) => {
      addNoteApi(formData.value)
    })

    uni.showToast({title: '文章发布成功', icon: 'success'})
    uni.switchTab({
      url: '/pages/index/index'
    })

  } catch {
    uni.showToast({title: '请填写完整信息', icon: 'error'})
    return
  }

}
const handleSelect = (e) => {
  const tempFilePaths = e.tempFilePaths;
  uni.uploadFile({
    fileType: 'image',
    filePath: tempFilePaths[0],
    url: 'http://localhost:3000/uploads',
    name: 'file',
    success: ({data, code}) => {
      formData.value.cover = JSON.parse(data).path

    },
    fail: (error) => {
      console.log(error)
    }
  });


}

</script>

<template>
  <view class="note">
    <uni-forms ref="formRef" :model="formData" :rules="rules" label-width="100">
      <uni-forms-item label="笔记标题" name="title" required>
        <uni-easyinput v-model="formData.title" placeholder="请输入笔记标题"/>
      </uni-forms-item>
      <uni-forms-item label="笔记内容" name="content" required>
        <uni-easyinput v-model="formData.content" :maxlength="-1" autoHeight type="textarea"/>
      </uni-forms-item>
      <uni-forms-item label="上传封面" name="cover" required>
        <uni-file-picker
            :limit="1"
            file-mediatype="image"
            @select="handleSelect"
        />
      </uni-forms-item>
    </uni-forms>
    <button type="primary" @click="submit">发布</button>


  </view>
</template>

<style lang="scss" scoped>
.note {
  padding: 20rpx;

  uni-easyinput {
    display: block;
    height: 200rpx;
  }

  button {
    width: 260rpx;
    height: 90rpx;
    line-height: 90rpx;
  }
}

</style>
