import { IsNotEmpty } from 'class-validator';

export class CreateArticleDto {
  @IsNotEmpty({ message: '文章标题不能为空' })
  readonly title: string;

  @IsNotEmpty({ message: '文章内容不能为空' })
  readonly content: string;

  @IsNotEmpty({ message: '文章封面不能为空' })
  readonly cover: string;
}
