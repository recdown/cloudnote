import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module';
import { ValidationPipe } from '@nestjs/common';
import { NestExpressApplication } from '@nestjs/platform-express';

async function bootstrap() {
  const app = await NestFactory.create<NestExpressApplication>(AppModule);
  app.useGlobalPipes(new ValidationPipe({ stopAtFirstError: true }));
  app.enableCors({
    origin: 'http://localhost:5173', // 允许前端地址跨域
    credentials: true,
  });
  app.useStaticAssets('uploads', { prefix: '/uploads' });
  await app.init();
  await app.listen(process.env.PORT ?? 3000);
}

bootstrap();
