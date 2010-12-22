//
//  TransparentWebViewAppDelegate.h
//  TransparentWebView
//
//  Created by Dirk van Oosterbosch on 22-12-10.
//  Copyright 2010 IR labs. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface TransparentWebViewAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
}

@property (assign) IBOutlet NSWindow *window;

@end
