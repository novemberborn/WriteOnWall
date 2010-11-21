
#import <Cocoa/Cocoa.h>

@interface WebViewWindow : NSWindow
{
   IBOutlet WebView* thisWebView;
}

- (void)setView:(WebView*)view onWindow: (NSWindow*) window;
@end
