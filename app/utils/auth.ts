import {onMounted} from 'vue';


/**
 * token校验钩子
 *
 */
function checkLogin() {

    const token = uni.getStorageSync('token')
    const pages = getCurrentPages()
    const currentPage = pages[pages.length - 1];

    if (token) {
        if (
            currentPage.route == 'pages/auth/auth' ||
            currentPage.route == 'pages/auth/register' ||
            currentPage.route == 'pages/auth/login') {

            uni.reLaunch({url: '/pages/index/index'})
            return true
        }

    } else {

        if (currentPage.route == 'pages/index/index' ||
            currentPage.route == 'pages/add/add' ||
            currentPage.route == 'pages/user/user') {
            uni.showToast({title: '请先登录', icon: 'error'})
            uni.reLaunch({url: '/pages/auth/auth'})
        }
        return false
    }


}


export default function useCheckLogin() {
    onMounted(() => {
        checkLogin();
    });
}
