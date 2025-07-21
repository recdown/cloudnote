import { HttpException, Injectable } from '@nestjs/common';
import { CreateArticleDto } from './dto/create-article.dto';

import { PrismaService } from '../prisma/prisma.service';
import { UpdateArticleDto } from './dto/update-article.dto';

@Injectable()
export class ArticleService {
  constructor(private readonly prismaService: PrismaService) {}

  async create(createArticleDto: CreateArticleDto, authorId: number) {
    try {
      await this.prismaService.article.create({
        data: {
          ...createArticleDto,
          authorId,
        },
      });

      return {
        code: 200,
        message: '创建文章成功',
      };
    } catch (error) {
      console.log(error);
      throw new HttpException('创建文章失败', 500);
    }
  }

  async findAll(id: number) {
    const res = await this.prismaService.article.findMany({
      where: { authorId: id },
      include: {
        author: {
          select: { username: true },
        },
      },
    });
    if (res) {
      return {
        code: 200,
        message: '获取文章列表成功',
        data: res,
      };
    } else {
      throw new HttpException(`获取文章列表失败`, 500);
    }
  }

  async findOne(id: number) {
    const res = await this.prismaService.article.findUnique({
      where: { id },
    });

    console.log('res内容:', res);
    if (res) {
      return {
        code: 200,
        message: '获取文章成功',
        data: res,
      };
    } else {
      throw new HttpException(`文章不存在,获取失败`, 400);
    }
  }

  async remove(number: number) {
    try {
      await this.prismaService.article.delete({
        where: { id: number },
      });
      return {
        code: 200,
        message: '删除文章成功',
      };
    } catch (error) {
      throw new HttpException(`删除文章失败:${error}`, 500);
    }
  }

  async update(number: number, updateArticleDto: UpdateArticleDto) {
    try {
      await this.prismaService.article.update({
        where: { id: number },
        data: updateArticleDto,
      });
      return {
        code: 200,
        message: '更新文章成功',
      };
    } catch (error) {
      throw new HttpException(`更新文章失败:${error}`, 500);
    }
  }
}
