import { Module } from '@nestjs/common';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { AuthModule } from './auth/auth.module';
import { JwtModule } from '@nestjs/jwt';
import { UserModule } from './user/user.module';
import { PrismaModule } from './prisma/prisma.module';
import { CryptoModule } from './crypto/crypto.module';

import { ArticleModule } from './article/article.module';
import { UploadsModule } from './uploads/uploads.module';

@Module({
  imports: [
    AuthModule,
    JwtModule.register({
      global: true,
      secret: 'secretKey',
      signOptions: { expiresIn: '7d' },
    }),
    UserModule,
    PrismaModule,
    CryptoModule,
    CryptoModule,
    ArticleModule,
    UploadsModule,
  ],
  controllers: [AppController],
  providers: [AppService],
})
export class AppModule {}
