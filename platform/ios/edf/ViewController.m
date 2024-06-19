//
//  ViewController.m
//  edf
//
//  Created by Manuel Cabrerizo on 18/06/2024.
//

#import "ViewController.h"

#include "game.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    const char *text = hello_from_game();
    NSLog(@"%@", [NSString stringWithUTF8String:text]);
    
}

@end
