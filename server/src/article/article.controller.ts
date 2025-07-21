import {
  Body,
  Controller,
  Delete,
  Get,
  Param,
  Post,
  Put,
  UseGuards,
} from '@nestjs/common';
import { ArticleService } from './article.service';
import { CreateArticleDto } from './dto/create-article.dto';

import { Guard } from '../gurad/guard.service';
import { GetTokenInfo } from '../dec/custom.decorator';
import { UpdateArticleDto } from './dto/update-article.dto';

@UseGuards(Guard)
@Controller('article')
export class ArticleController {
  constructor(private readonly articleService: ArticleService) {}

  @Post()
  create(
    @Body() createArticleDto: CreateArticleDto,
    @GetTokenInfo('id') id: number,
  ) {
    return this.articleService.create(createArticleDto, +id);
  }

  @Get()
  findAll(@GetTokenInfo('id') id: number) {
    return this.articleService.findAll(+id);
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.articleService.findOne(+id);
  }

  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.articleService.remove(+id);
  }

  @Put(':id')
  update(@Param('id') id: string, @Body() updateArticleDto: UpdateArticleDto) {
    return this.articleService.update(+id, updateArticleDto);
  }
}
