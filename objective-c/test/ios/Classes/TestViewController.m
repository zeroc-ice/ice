// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <TestViewController.h>
#import <Test.h>
#import <AppDelegate.h>

#import <TestCommon.h>

//
// Avoid warning for undocumented method.
//
@interface UIApplication(UndocumentedAPI)
-(void)launchApplicationWithIdentifier:(NSString*)id suspended:(BOOL)flag;
@end

// TODO: Would be nice to have a red font for fatal, and error messages.
@interface MessageCell : UITableViewCell
{
@private
    UILabel* body;
}

@property (nonatomic, retain) UILabel* body;

+(CGFloat)heightForMessage:(NSString*)messsage;

@end

@implementation MessageCell
@synthesize body;

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    if (self = [super initWithStyle:style reuseIdentifier:reuseIdentifier])
    {
        body = [[UILabel alloc] initWithFrame:CGRectZero];
        body.textColor = [UIColor blackColor];
        body.font = [UIFont boldSystemFontOfSize:14];
        body.numberOfLines = 0;
        
        [self.contentView addSubview:self.body];
    }
    
    return self;
}

+(CGFloat)heightForMessage:(NSString*)text
{
    // The header is always one line, the body is multiple lines.
    // The width of the table is 320 - 20px of left & right padding. We don't want to let the body
    // text go past 200px.
    CGRect body = [text boundingRectWithSize:CGSizeMake(300.f, 200.0f) options:NSStringDrawingUsesLineFragmentOrigin|NSStringDrawingUsesFontLeading attributes:@{NSFontAttributeName:[UIFont boldSystemFontOfSize:14] } context:nil];
    return body.size.height + 20.f;
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    
    CGRect contentRect = self.contentView.bounds;
    
    CGRect bodyFrame = CGRectMake(10.f, 0.f, CGRectGetWidth(contentRect)-20.f, CGRectGetHeight(contentRect));
    
    self.body.frame = bodyFrame;
}

#if defined(__clang__) && !__has_feature(objc_arc)
- (void)dealloc
{
    [body release];
    [super dealloc];
}
#endif

-(void)setMessage:(NSString*)m
{
    self.body.text = m;
}

@end

@interface TestViewController()

@property (nonatomic, retain) UITableView* output;
@property (nonatomic, retain) UIActivityIndicatorView* activity;
@property (nonatomic, retain) UIButton* nextButton;

@property (nonatomic, retain) NSMutableString* currentMessage;
@property (nonatomic, retain) NSMutableArray* messages;
@property (nonatomic, retain) NSOperationQueue* queue;
@property (retain) Test* test;

-(void)add:(NSString*)d;
-(void)startTest;
@end

@interface TestRun : NSObject
{
    TestConfigOption option;
    Test* test;
    int completed;
    int error;
    TestViewController* viewController;
}

+(id) testRun:(Test*)test;
+(id) testRunWithConfigOption:(Test*)test option:(TestConfigOption)option;

-(NSInvocationOperation*) runInvocation:(TestViewController*)callback;
@end

@implementation TestViewController

@synthesize output;
@synthesize activity;
@synthesize nextButton;
@synthesize currentMessage;
@synthesize messages;
@synthesize queue;
@synthesize test;

- (void)viewDidLoad
{
    self.currentMessage = [NSMutableString string];
    self.messages = [NSMutableArray array];
    queue = [[NSOperationQueue alloc] init];
    self.queue.maxConcurrentOperationCount = 2; // We need at least 2 concurrent operations.
    TestCommonInit(self, @selector(add:));
    [super viewDidLoad];
}

-(void)viewWillAppear:(BOOL)animated
{
    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    self.test = (Test*)[appDelegate.tests objectAtIndex:appDelegate.currentTest];
    [self startTest];
}

-(void)viewWillDisappear:(BOOL)animated
{
    // TODO: Waiting isn't possible until the tests periodically find out whether
    // they should terminate.
    // Wait until the tests are complete.
    //[queue waitUntilAllOperationsAreFinished];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}

#if defined(__clang__) && !__has_feature(objc_arc)
- (void)dealloc
{
    [output release];
    [activity release];
    [nextButton release];
    
    [currentMessage release];
    [messages release];
    [queue release];
    [test release];

    [super dealloc];
}
#endif

#pragma mark -

-(void)startTest
{
    self.title = test.name;
    [self.navigationItem setHidesBackButton:YES animated:YES];

    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    if(appDelegate.loop)
    {
        [nextButton setTitle:@"Stop running" forState:UIControlStateNormal];
    }
    else
    {
        nextButton.enabled = NO;
        [nextButton setAlpha:0.5];
        [nextButton setTitle:@"Test is running" forState:UIControlStateDisabled];
    }

    // Clear the current message, and the the table log.
    [currentMessage deleteCharactersInRange:NSMakeRange(0, currentMessage.length)];
    [messages removeAllObjects];
    [output reloadData];
    [activity startAnimating];

    if(![test isProtocolSupported:appDelegate.protocol])
    {
        tprintf("Test not supported with %s\n", [appDelegate.protocol UTF8String]);
        [self testComplete:YES];
        return;
    }

    NSMutableArray* testRuns = [NSMutableArray array];
    [testRuns addObject:[TestRun testRun:test]];
    if(test.runWithSlicedFormat)
    {
        [testRuns addObject:[TestRun testRunWithConfigOption:test option:TestConfigOptionSliced]];
    }
    if(test.runWith10Encoding)
    {
        [testRuns addObject:[TestRun testRunWithConfigOption:test option:TestConfigOptionEncoding10]];
    }
    testRunEnumerator = [testRuns objectEnumerator];
#if defined(__clang__) && !__has_feature(objc_arc)
    [testRunEnumerator retain];
#endif
    id testRun = [testRunEnumerator nextObject];
    [queue addOperation:[testRun runInvocation:self]];
}

-(void)testRunComplete:(BOOL)success
{
    if(!success)
    {
        [self testComplete:NO];
        return;
    }
    
    id testRun = [testRunEnumerator nextObject];
    if(testRun == nil)
    {
        [self testComplete:YES];
    }
    else
    {
        [queue addOperation:[testRun runInvocation:self]];
    }
}

-(void)testComplete:(BOOL)success
{
    [activity stopAnimating];

    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    Test* nextTest = (Test*)[appDelegate.tests objectAtIndex:(appDelegate.currentTest+1)%(appDelegate.tests.count)];
    NSString* buttonTitle = [NSString stringWithFormat:@"Run %@", nextTest.name];
    [nextButton setTitle:buttonTitle forState:UIControlStateNormal];

    nextButton.enabled = YES;
    [nextButton setAlpha:1.0];
    [self.navigationItem setHidesBackButton:NO animated:YES];
    
    self.test = nil;
#if defined(__clang__) && !__has_feature(objc_arc)
    [testRunEnumerator release];
#endif
    testRunEnumerator = nil;

    // For memory leak testing comment the following line out.
    if([appDelegate testCompleted:success])
    {
        self.test = (Test*)[appDelegate.tests objectAtIndex:appDelegate.currentTest];
        [self startTest];
    }
}

-(IBAction)next:(id)sender
{
    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    if(appDelegate.loop)
    {
        appDelegate.loop = NO;
        nextButton.enabled = NO;
        [nextButton setAlpha:0.5];
        [nextButton setTitle:@"Waiting..." forState:UIControlStateDisabled];
    }
    else
    {
        NSAssert(test == nil, @"test == nil");
        AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
        self.test = (Test*)[appDelegate.tests objectAtIndex:appDelegate.currentTest];
        [self startTest];
    }
}

-(NSOperationQueue*) queue
{
    return queue;
}

-(void)add:(NSString*)s
{
    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    if(appDelegate.runAll)
    {
        printf("%s", [s UTF8String]);
    }

    [currentMessage appendString:s];
    NSRange range = [currentMessage rangeOfString:@"\n" options:NSBackwardsSearch];
    if(range.location != NSNotFound)
    {
#if defined(__clang__) && !__has_feature(objc_arc)
        [messages addObject:[[currentMessage copy] autorelease]];
#else
        [messages addObject:[currentMessage copy]];
#endif
        [currentMessage deleteCharactersInRange:NSMakeRange(0, currentMessage.length)];
        [output reloadData];
        NSUInteger path[] = {0, messages.count-1};
        [output scrollToRowAtIndexPath:[NSIndexPath indexPathWithIndexes:path length:2]
                      atScrollPosition:UITableViewScrollPositionBottom
                              animated:NO];
    }
}

#pragma mark <UITableViewDelegate, UITableViewDataSource> Methods

-(NSInteger)numberOfSectionsInTableView:(UITableView *)tv
{
    return 1;
}

-(NSInteger)tableView:(UITableView *)tv numberOfRowsInSection:(NSInteger)section
{
    return messages.count;
}

-(CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if([messages count] <= indexPath.row)
    {
        return [MessageCell heightForMessage:@""];
    }
    return [MessageCell heightForMessage:[messages objectAtIndex:indexPath.row]];
}

-(UITableViewCell *)tableView:(UITableView *)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    MessageCell *cell = (MessageCell*)[output dequeueReusableCellWithIdentifier:@"MessageCell"];
    if(cell == nil)
    {
#if defined(__clang__) && !__has_feature(objc_arc)
        cell = [[[MessageCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"MessageCell"] autorelease];
#else
        cell = [[MessageCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"MessageCell"];
#endif
    }
    [cell setMessage:[messages objectAtIndex:indexPath.row]];
    return cell;
}

-(NSIndexPath *)tableView:(UITableView *)tableView willSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    return nil;
}

@end

@implementation TestRun
-(id) init:(Test*)t option:(TestConfigOption)opt
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->test = t;
    self->option = opt;
    self->completed = 0;
    self->error = 0;
    return self;
}

+(id) testRun:(Test*)test
{
    TestRun* run = [[TestRun alloc] init:test option:TestConfigOptionDefault];
#if defined(__clang__) && !__has_feature(objc_arc)
    [run autorelease];
#endif
    return run;
}
+(id) testRunWithConfigOption:(Test*)test option:(TestConfigOption)option
{
    TestRun* run = [[TestRun alloc] init:test option:option];
#if defined(__clang__) && !__has_feature(objc_arc)
    [run autorelease];
#endif
    return run;
}

-(NSInvocationOperation*) runInvocation:(TestViewController*)ctl
{
    viewController = ctl;
#if defined(__clang__) && !__has_feature(objc_arc)
    return [[[NSInvocationOperation alloc] initWithTarget:self selector:@selector(run) object:nil] autorelease];
#else
    return [[NSInvocationOperation alloc] initWithTarget:self selector:@selector(run) object:nil];
#endif

}

-(void) run
{
    // Initialize testCommon.
    AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
    if(appDelegate.runAll)
    {
        printf("\n*** running %s test %ld/%lu ...\n", [test.name UTF8String], (long)appDelegate.currentTest + 1,
               (unsigned long)[appDelegate.tests count]);
        printf("*** protocol: %s\n", [appDelegate.protocol UTF8String]);
        fflush(stdout);
    }

    TestCommonTestInit(self, 
                       @selector(serverReady),
                       appDelegate.protocol,
                       option == TestConfigOptionSliced,
                       option == TestConfigOptionEncoding10);
    
    if(option == TestConfigOptionSliced)
    {
        tprintf("Running test with %s and sliced format.\n", [appDelegate.protocol UTF8String]);
    }
    else if(option == TestConfigOptionEncoding10)
    {
        tprintf("Running test with %s and 1.0 encoding.\n", [appDelegate.protocol UTF8String]);
    }
    else
    {
        tprintf("Running test with %s and default format.\n", [appDelegate.protocol UTF8String]);
    }

    if([test hasServer])
    {
        [self runServer];
    }
    else
    {
        [self runClient];
    }
}

-(void)clientComplete:(NSNumber*)rc
{
    if([rc intValue] != 0)
    {
        [viewController add:[NSString stringWithFormat:@"client error: %@!\n", rc]];
        if([test hasServer])
        {
            serverStop();
        }
        ++error;
    }
    
    if(![test hasServer] || ++completed == 2)
    {
        [viewController testRunComplete:error == 0];
    }
}

// Run in a separate thread.
-(void)runClient
{
    int rc = [test client];
    [self performSelectorOnMainThread:@selector(clientComplete:) withObject:[NSNumber numberWithInt:rc] waitUntilDone:NO];
}

-(void)serverComplete:(NSNumber*)rc
{
    if([rc intValue] != 0)
    {
        [viewController add:[NSString stringWithFormat:@"server error: %@!\n", rc]];
        ++error;
    }
    
    if(++completed == 2)
    {
        [viewController testRunComplete:error == 0];
    }
}

// Run in a separate thread.
-(void)runServer
{
    int rc = [test server];
    [self performSelectorOnMainThread:@selector(serverComplete:) withObject:[NSNumber numberWithInt:rc] waitUntilDone:NO];
}

// Kick off the client.
-(void)serverReady
{
    NSInvocationOperation* invocation = [[NSInvocationOperation alloc]
                                         initWithTarget:self
                                         selector:@selector(runClient)
                                         object:nil];
#if defined(__clang__) && !__has_feature(objc_arc)
    [invocation autorelease];
#endif
    [[viewController queue] addOperation:invocation];
}
@end


