import { HttpException, Injectable } from '@nestjs/common';

import { LoginAuthDto } from './dto/login-auth.dto';
import { JwtService } from '@nestjs/jwt';
import { CreateAuthDto } from './dto/create-auth.dto';
import { PrismaService } from 'src/prisma/prisma.service';
import { CryptoService } from '../crypto/crypto.service';

@Injectable()
export class AuthService {
  constructor(
    private readonly jwtService: JwtService,
    private readonly prismaService: PrismaService,
    private readonly cryptoService: CryptoService,
  ) {}

  /**
   * 用户注册
   * @param createAuthDto
   */
  async register(createAuthDto: CreateAuthDto) {
    const res = await this.prismaService.user.findFirst({
      where: {
        username: createAuthDto.username,
      },
    });

    if (res) throw new HttpException('用户名已存在', 402);

    const salt = this.cryptoService.generateSalt();
    const password = this.cryptoService.hashPassword(
      createAuthDto.password,
      salt,
    );
    const user = await this.prismaService.user.create({
      data: {
        username: createAuthDto.username,
        password,
        salt,
      },
    });
    if (user)
      return {
        code: 200,
        data: {
          status: '注册成功',
          id: user.id,
          username: user.username,
        },
        message: 'success',
      };

    return {
      code: 402,
      data: {
        status: '注册失败',
      },
      message: 'fail',
    };
  }

  /**
   * 用户登录
   * @param loginAuthDto
   */
  async login(loginAuthDto: LoginAuthDto) {
    const user = await this.prismaService.user.findFirst({
      where: {
        username: loginAuthDto.username,
      },
    });

    if (!user) throw new HttpException('用户名不存在', 402);

    if (
      this.cryptoService.hashPassword(loginAuthDto.password, user.salt) !==
      user.password
    ) {
      throw new HttpException('密码错误', 402);
    }
    return {
      code: 200,
      token: this.jwtService.sign({ id: user.id, username: user.username }),
    };
  }
}
