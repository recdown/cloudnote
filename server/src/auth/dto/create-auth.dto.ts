import { IsNotEmpty, MinLength } from 'class-validator';

export class CreateAuthDto {
  @MinLength(5, { message: '用户名不能少于5个字符' })
  @IsNotEmpty({ message: '用户名不能为空' })
  username: string;

  @MinLength(8, { message: '密码不能少于8个字符' })
  @IsNotEmpty({ message: '密码不能为空' })
  password: string;
  salt: string;
}
