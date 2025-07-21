import {NoteDTO} from '../dto/note.dto'
import http from '@/utils/http'


export const addNoteApi = (data: NoteDTO) => http.post('article', data)
export const getNoteListApi = () => http.get('article')
export const getNoteApi = (id: number) => http.get(`article/${id}`)
export const deleteNoteApi = (id: number) => http.delete(`article/${id}`)
export const updateNoteApi = (id: number, data: NoteDTO) => http.put(`article/${id}`, data)
