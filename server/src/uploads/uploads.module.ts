import { Global, Module } from '@nestjs/common';

import { UploadsController } from './uploads.controller';
import { MulterModule } from '@nestjs/platform-express';
import { diskStorage } from 'multer';

@Global()
@Module({
  imports: [
    MulterModule.registerAsync({
      useFactory: () => ({
        storage: diskStorage({
          destination: './uploads',
          filename(req, file, cb) {
            cb(null, `${Date.now()}_${file.originalname}`);
          },
        }),
      }),
    }),
  ],
  controllers: [UploadsController],
  providers: [],
})
export class UploadsModule {}
