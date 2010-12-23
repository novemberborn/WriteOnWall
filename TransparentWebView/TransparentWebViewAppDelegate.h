//
//  TransparentWebViewAppDelegate.h
//  TransparentWebView
//
//  Created by Dirk van Oosterbosch on 22-12-10.
//  Copyright 2010 IR labs. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <WebKit/WebView.h>
#import <WebKit/WebFrame.h>

extern NSString *const TWVLocationUrlKey;


@interface TransparentWebViewAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
	WebView *theWebView;
	
	NSWindow *locationSheet;
	NSString *urlString;
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet WebView *theWebView;

@property (assign) IBOutlet NSWindow *locationSheet;
@property (nonatomic, retain) NSString *urlString;

//- (IBAction)reloadPage:(id)sender;

- (IBAction)showLocationSheet:(id)sender;
- (IBAction)endLocationSheet:(id)sender;
- (IBAction)cancelLocationSheet:(id)sender;

- (void)loadUrlString:(NSString *)anUrlString IntoWebView:(WebView *)aWebView;

@end
