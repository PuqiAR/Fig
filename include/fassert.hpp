#pragma once

#define fassert(exp,msg) (if(!exp) throw msg;)