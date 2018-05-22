//
//  main.cpp
//  PB2JSON
//
//  Created by littleliang on 2018/4/28.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#include <stdio.h>
#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/compiler/objectivec/objectivec_generator.h>


int main(int argc, char *argv[]) {
//    ProtobufMsg message;
    
    argc = 5;

    char **argv1 = new char *[3];
    argv1[0] = new char[200];
    strcpy(argv1[0], argv[0]);

    argv1[1] = new char[200];
    strcpy(argv1[1], "./BaseResponse.proto");
//
    argv1[2] = new char[200];
    strcpy(argv1[2], "./TargetResponse.proto");
//
    
    argv1[3] = new char[200];
    strcpy(argv1[3], "--target=CgiF2FPaySucPageResp");
    
    argv1[4] = new char[200];
    strcpy(argv1[4], "--mockcase_out=./");
    
    google::protobuf::compiler::CommandLineInterface cli;
    
    google::protobuf::compiler::objectivec::ObjectiveCGenerator objc_generator;
    cli.RegisterGenerator("--mockcase_out", &objc_generator, "Generate Mock Case Script File");
    
    return cli.Run(argc, argv1);
}
