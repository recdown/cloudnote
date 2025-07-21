import http from '@/utils/http'


export const getUserApi = () => http.get('user')
