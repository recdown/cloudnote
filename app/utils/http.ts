import Request from '../js_sdk/luch-request/luch-request/index.js'

const http = new Request({
    baseURL: 'http://localhost:3000',
    timeout: 10000,
    header: {
        'Content-Type': 'application/json'
    }
})

// 请求拦截器
http.interceptors.request.use((config) => {
    const token = uni.getStorageSync('token')
    if (token) {
        config.header.Authorization = `${token}`
    }

    return config
}, (error) => {
    return Promise.reject(error)
})

// 响应拦截器
http.interceptors.response.use((response) => {

    // if (response.data.code !== 200) {
    //   uni.showToast({ title: response.data.message || '请求失败', icon: 'none' })
    //   return Promise.reject(response.data)
    // }
    return response.data
}, (error) => {
    uni.showToast({title: error.data.message, icon: 'none'})
    return Promise.reject(error)
})

export default http
