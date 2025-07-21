import {
  applyDecorators,
  createParamDecorator,
  ExecutionContext,
  UseInterceptors,
} from '@nestjs/common';
import { Request } from 'express';
import { FileInterceptor } from '@nestjs/platform-express';
import { MulterOptions } from '@nestjs/platform-express/multer/interfaces/multer-options.interface';

export const GetTokenInfo = createParamDecorator(
  (data: string, ctx: ExecutionContext) => {
    const request: Request = ctx.switchToHttp().getRequest();

    return data
      ? request.user?.[data as keyof typeof request.user]
      : request.user;
  },
);

export const Uploads = (fileName = 'file', options: MulterOptions) => {
  return applyDecorators(UseInterceptors(FileInterceptor(fileName, options)));
};
