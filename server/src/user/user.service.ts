import { Injectable } from '@nestjs/common';

import { PrismaService } from '../prisma/prisma.service';

@Injectable()
export class UserService {
  constructor(private readonly prismaService: PrismaService) {}

  findAll() {
    return this.prismaService.user.findMany();
  }

  async findOne(id: number) {
    return this.prismaService.user.findUnique({
      where: {
        id,
      },
    });
  }
}
