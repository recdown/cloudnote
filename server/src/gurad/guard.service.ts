import {
  CanActivate,
  ExecutionContext,
  HttpException,
  Injectable,
} from '@nestjs/common';
import { Observable } from 'rxjs';
import { JwtService } from '@nestjs/jwt';
import { Request } from 'express';

declare module 'express' {
  interface Request {
    user?: {
      id: number;
      username: string;
    };
  }
}

@Injectable()
export class Guard implements CanActivate {
  constructor(private readonly jwtService: JwtService) {}

  canActivate(
    context: ExecutionContext,
  ): boolean | Promise<boolean> | Observable<boolean> {
    const request: Request = context.switchToHttp().getRequest();

    const token = request.headers.authorization;

    try {
      request.user = this.jwtService.verify<{ id: number; username: string }>(
        <string>token,
        {
          secret: 'secretKey',
        },
      );
    } catch (error) {
      console.log(error);
      throw new HttpException('登录状态异常,请重试', 401);
    }
    return true;
  }
}
