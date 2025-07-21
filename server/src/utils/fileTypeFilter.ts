import { Request } from 'express';
import { FileFilterCallback } from 'multer';
import { HttpException } from '@nestjs/common';

/**
 * 创建一个文件类型过滤器，用于 multer 的 fileFilter
 * @param fileType 文件类型，例如 image/jpeg
 * @returns 一个函数，用于 multer 的 fileFilter
 */
export const fileTypeFilter = (...fileType: string[]) => {
  return (req: Request, file: Express.Multer.File, cb: FileFilterCallback) => {
    if (fileType.some((item) => file.mimetype.includes(item))) {
      cb(null, true);
    } else {
      cb(new HttpException('文件类型错误', 400));
    }
  };
};
