#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"

// 群组用户多了一个role角色信息，从User类直接继承，复用User类的属性
class GroupUser : public User
{
public:
    void setRole(string role) { this->role = role; }
    string getRole() { return this->role; }

private:
    // 群组用户的角色
    string role;
};

#endif // GROUPUSER_H