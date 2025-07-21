<script lang="ts" setup>
import {ref} from 'vue'
import {onLoad} from '@dcloudio/uni-app'
import {NoteDTO} from '../../dto/note.dto'
import {getNoteApi, updateNoteApi} from '../../api/article'
import useCheckLogin from '@/utils/auth'

useCheckLogin()

const formData = ref<NoteDTO>({
  title: '',
  content: '',
  cover: ''
})


const fileList = ref<any[]>([])


const ids = ref<number>()


const handleSelect = (e: any) => {
  const tempFilePaths = e.tempFilePaths
  uni.uploadFile({
    fileType: 'image',
    filePath: tempFilePaths[0],
    url: 'http://localhost:3000/uploads',
    name: 'file',
    success: ({data}) => {
      // 服务器返回的路径，可能是 Windows 风格，替换成 /
      const rawPath = JSON.parse(data).path
      const path = rawPath.replace(/\\/g, '/')
      // 更新 formData 和 fileList，实现回显
      formData.value.cover = path
      fileList.value = [
        {
          url: 'http://localhost:3000/' + path,
        }
      ]
    },
    fail: (error) => {
      console.error('上传失败', error)
    }
  })
}


const getNote = async () => {
  if (!ids.value) return
  const res = await getNoteApi(ids.value)
  const {title, content, cover} = res.data


  const fixedCover = cover.replace(/\\/g, '/')

  formData.value = {
    title,
    content,
    cover: fixedCover
  }

  if (fixedCover) {
    fileList.value = [
      {
        url: 'http://localhost:3000/' + fixedCover,
        name: '封面图.jpg',
        extname: 'jpg'
      }
    ]
  } else {
    fileList.value = []
  }
}

// 提交更新
const submit = async () => {
  if (!ids.value) return
  await updateNoteApi(ids.value, formData.value)
  uni.navigateBack()
  uni.showToast({
    title: '发布成功',
    icon: 'success'
  })
}


onLoad((options: any) => {
  ids.value = Number(options.id)
  getNote()

})
</script>

<template>
  <view class="note">
    <uni-forms :model="formData" label-width="100">
      <uni-forms-item label="笔记标题" name="title">
        <uni-easyinput v-model="formData.title" placeholder="请输入笔记标题"/>
      </uni-forms-item>
      <uni-forms-item label="笔记内容" name="content">
        <uni-easyinput
            v-model="formData.content"
            :maxlength="-1"
            autoHeight
            placeholder="请输入笔记内容"
            type="textarea"
        />
      </uni-forms-item>
      <uni-forms-item label="上传封面" name="cover">
        <uni-file-picker
            v-model="fileList"
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
    margin-top: 20rpx;
  }
}
</style>
