export const rules = {
    username: {
        rules: [
            {required: true, errorMessage: '请输入用户名'},
            {minLength: 5, errorMessage: '用户名至少5个字符'},
            {maxLength: 12, errorMessage: '用户名最多12个字符'},
        ]
    },
    password: {
        rules: [
            {required: true, errorMessage: '请输入密码'},
            {minLength: 8, errorMessage: '密码至少8个字符'},
            {maxLength: 20, errorMessage: '密码最多20个字符'}
        ]
    }
}
