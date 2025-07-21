import http from '@/utils/http'
import {LoginDTO, RegisterDTO} from '@/dto/auth.dto'

export const loginApi = (data: LoginDTO) => {
    return http.post('auth/login', data)
}

export const registerApi = (data: RegisterDTO) => {
    return http.post('auth/register', data)
}
