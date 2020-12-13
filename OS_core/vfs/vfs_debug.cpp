//
// Created by shinsya on 2020/12/11.
//

#include <stdio.h>
#include <imgfs/imgfs_struct.h>

int main(){
    printf("%d\n", imgfs_pre_dir_level("./var/a.txt"));
    printf("%d\n", imgfs_pre_dir_level("/var/a.txt"));
    printf("%d\n", imgfs_pre_dir_level("./var/"));
    printf("%d\n", imgfs_pre_dir_level("/var/"));
    printf("%d\n", imgfs_pre_dir_level("/var"));
    printf("%d\n", imgfs_pre_dir_level("/var/../root/a.txt"));
    return 0;
}
