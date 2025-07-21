import { Controller, Post, UploadedFile } from '@nestjs/common';
import { fileTypeFilter } from '../utils/fileTypeFilter';
import { Uploads } from '../dec/custom.decorator';

@Controller('uploads')
export class UploadsController {
  @Post()
  @Uploads('file', { fileFilter: fileTypeFilter('image') })
  uploadFile(@UploadedFile() file: Express.Multer.File) {
    return file;
  }
}
