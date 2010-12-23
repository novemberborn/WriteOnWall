//
//  TransparentWebViewAppDelegate.m
//  TransparentWebView
//
//  Created by Dirk van Oosterbosch on 22-12-10.
//  Copyright 2010 IR labs. All rights reserved.
//

#import "TransparentWebViewAppDelegate.h"

NSString *const TWVLocationUrlKey = @"WebViewLocationUrl";

@implementation TransparentWebViewAppDelegate

@synthesize window, theWebView;
@synthesize locationSheet, urlString;


- (id) init {
	[super init];
	
	// Register the Defaults in the Preferences
	NSMutableDictionary *defaultValues = [NSMutableDictionary dictionary];
	[defaultValues setObject:@"http://localhost/" forKey:TWVLocationUrlKey];
	[[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
	
	// Set the url from the Preferences file
	self.urlString = [[NSUserDefaults standardUserDefaults] objectForKey:TWVLocationUrlKey];
	
	return self;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Insert code here to initialize your application
	NSLog(@"TransparentWebView app got launched ...");
	[self loadUrlString:self.urlString IntoWebView:self.theWebView];
}

- (IBAction)showLocationSheet:(id)sender {
	//
	[NSApp beginSheet:locationSheet
	   modalForWindow:window
		modalDelegate:nil
	   didEndSelector:NULL
		  contextInfo:NULL];
}


- (IBAction)endLocationSheet:(id)sender {
	
	// Return to normal event handling and hide the sheet
	[NSApp endSheet:locationSheet];
	[locationSheet orderOut:sender];
	
	// Save the location url in the Preferences
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	[defaults setObject:self.urlString forKey:TWVLocationUrlKey];
	
	NSLog(@"Load the url: %@", urlString);
	[self loadUrlString:self.urlString IntoWebView:self.theWebView];
}

- (IBAction)cancelLocationSheet:(id)sender {
	// Return to normal event handling and hide the sheet
	[NSApp endSheet:locationSheet];
	[locationSheet orderOut:sender];
}


/*
 * The method to load any url string into a web view of choice
 */
- (void)loadUrlString:(NSString *)anUrlString IntoWebView:(WebView *)aWebView {
	
	// Make an URL from the String, and then a Request from the URL
	NSURL *url = [NSURL URLWithString:anUrlString];
	NSURLRequest *urlReq = [NSURLRequest requestWithURL:url];
	
	// Get the webFrame and load the request
	WebFrame* webFrame = [aWebView mainFrame];
	[webFrame loadRequest: urlReq];
}

@end
