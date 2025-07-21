import { Controller, Get, UseGuards } from '@nestjs/common';
import { UserService } from './user.service';

import { Guard } from '../gurad/guard.service';
import { GetTokenInfo } from '../dec/custom.decorator';

@UseGuards(Guard)
@Controller('user')
export class UserController {
  constructor(private readonly userService: UserService) {}

  // @Get()
  // findAll() {
  //   return this.userService.findAll();
  // }

  @Get()
  findOne(@GetTokenInfo('id') id: string) {
    return this.userService.findOne(+id);
  }
}
