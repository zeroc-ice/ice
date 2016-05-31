// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <TestViewController.h>
#import <TestUtil.h>
#import <AppDelegate.h>
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

- (void)dealloc
{
    [body release];
    [super dealloc];
}

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
@property (retain) TestCase* test;

-(void)add:(NSString*)d;
-(void)startTest;
@end

@interface TestRun : NSObject
{
    TestConfigOption option;
    TestCase* test;
    NSString* server;
    NSString* client;
    BOOL collocated;
    
    int completed;
    int running;
    int error;
    MainHelperI* clientHelper;
    MainHelperI* serverHelper;
    TestViewController* viewController;
}

+(id) testRunClient:(TestCase*)test;
+(id) testRunCollocated:(TestCase*)test;
+(id) testRunClientAMDServer:(TestCase*)test;
+(id) testRunClientServer:(TestCase*)test;
+(id) testRunClientServerWithConfigOption:(TestCase*)test option:(TestConfigOption)option;
+(id) testRunClientAMDServerWithConfigOption:(TestCase*)test option:(TestConfigOption)option;

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
    
    [super viewDidLoad];
}

-(void)viewWillAppear:(BOOL)animated
{
    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    self.test = (TestCase*)[appDelegate.tests objectAtIndex:appDelegate.currentTest];
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
    
    [currentMessage deleteCharactersInRange:NSMakeRange(0, currentMessage.length)];
    [messages removeAllObjects];
    [output reloadData];
    
    [activity startAnimating];
    if(![test isProtocolSupported:appDelegate.protocol])
    {
        [self testComplete:YES];
        return;
    }
#ifdef ICE_CPP11_MAPPING
    if(!test.cpp11Support)
    {
        [self add:@"C++11 not supported by this test\n"];
        [self testComplete:YES];
        return;
    }
#endif
    
    NSMutableArray* testRuns = [NSMutableArray array];
    if([test hasServer])
    {
        [testRuns addObject:[TestRun testRunClientServer:test]];
        if([test hasAMDServer])
        {
            [testRuns addObject:[TestRun testRunClientAMDServer:test]];
        }
        if(test.runWithSlicedFormat)
        {
            [testRuns addObject:[TestRun testRunClientServerWithConfigOption:test option:TestConfigOptionSliced]];
            if([test hasAMDServer])
            {
                [testRuns addObject:[TestRun testRunClientAMDServerWithConfigOption:test option:TestConfigOptionSliced]];
            }
        }
        if(test.runWith10Encoding)
        {
            [testRuns addObject:[TestRun testRunClientServerWithConfigOption:test option:TestConfigOptionEncoding10]];
            if([test hasAMDServer])
            {
                [testRuns addObject:[TestRun testRunClientAMDServerWithConfigOption:test option:TestConfigOptionEncoding10]];
            }
        }
    }
    else
    {
        [testRuns addObject:[TestRun testRunClient:test]];
    }
    if([test hasCollocated])
    {
        [testRuns addObject:[TestRun testRunCollocated:test]];
    }
    
    testRunEnumator = [[testRuns objectEnumerator] retain];
    id testRun = [testRunEnumator nextObject];
    [queue addOperation:[testRun runInvocation:self]];
}
-(void)testRunComplete:(BOOL)success
{
    if(!success)
    {
        [self testComplete:NO];
        return;
    }
    
    id testRun = [testRunEnumator nextObject];
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

    nextButton.enabled = YES;
    [nextButton setAlpha:1.0];
    [self.navigationItem setHidesBackButton:NO animated:YES];

    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    TestCase* nextTest = (TestCase*)[appDelegate.tests objectAtIndex:(appDelegate.currentTest+1)%(appDelegate.tests.count)];
    NSString* buttonTitle = [NSString stringWithFormat:@"Run %@", nextTest.name];
    [nextButton setTitle:buttonTitle forState:UIControlStateNormal];
    self.test = nil;
    [testRunEnumator release];
    testRunEnumator = nil;
    
    if([appDelegate testCompleted:success])
    {
        NSAssert(test == nil, @"test == nil");
        self.test = (TestCase*)[appDelegate.tests objectAtIndex:appDelegate.currentTest];
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
        self.test = (TestCase*)[appDelegate.tests objectAtIndex:appDelegate.currentTest];
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
        [messages addObject:[[currentMessage copy] autorelease]];
        [currentMessage deleteCharactersInRange:NSMakeRange(0, currentMessage.length)];

        // reloadData hangs if called too rapidly... we call at most every 100ms.
        if(!reloadScheduled) {
            reloadScheduled = TRUE;
            [self performSelector:@selector(reloadOutput) withObject:nil afterDelay:0.1];
        }
    }
}
-(void) reloadOutput
{
    reloadScheduled = FALSE;
    [output reloadData];

    if(messages.count == 0)
    {
        return;
    }

    NSUInteger path[] = {0, messages.count -1};
    [output scrollToRowAtIndexPath:[NSIndexPath indexPathWithIndexes:path length:2]
                  atScrollPosition:UITableViewScrollPositionBottom
                          animated:NO];

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
        cell = [[[MessageCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"MessageCell"] autorelease];
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
-(id) init:(TestCase*)t client:(NSString*)cl server:(NSString*)srv option:(TestConfigOption)opt
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->test = t;
    self->client = cl;
    self->server = srv;
    self->option = opt;
    self->clientHelper = 0;
    self->serverHelper = 0;
    self->completed = 0;
    self->error = 0;
    self->running = 0;
    self->collocated = NO;
    return self;
}

+(id) testRunCollocated:(TestCase*)test
{
    TestRun* r;
    r = [[[TestRun alloc] init:test client:test.collocated server:nil option:TestConfigOptionDefault] autorelease];
    r->collocated = YES;
    return r;
}
+(id) testRunClient:(TestCase*)test
{
    return [[[TestRun alloc] init:test client:test.client server:nil option:TestConfigOptionDefault] autorelease];
}
+(id) testRunClientServer:(TestCase*)test
{
    return [[[TestRun alloc] init:test client:test.client server:test.server option:TestConfigOptionDefault] autorelease];
}
+(id) testRunClientAMDServer:(TestCase*)test
{
    return [[[TestRun alloc] init:test client:test.client server:test.serveramd option:TestConfigOptionDefault] autorelease];
}
+(id) testRunClientServerWithConfigOption:(TestCase*)test option:(TestConfigOption)option
{
    return [[[TestRun alloc] init:test client:test.client server:test.server option:option] autorelease];
}
+(id) testRunClientAMDServerWithConfigOption:(TestCase*)test option:(TestConfigOption)option
{
    return [[[TestRun alloc] init:test client:test.client server:test.serveramd option:option] autorelease];
}

-(NSInvocationOperation*) runInvocation:(TestViewController*)ctl
{
    viewController = ctl;
    return [[[NSInvocationOperation alloc] initWithTarget:self selector:@selector(run) object:nil] autorelease];
}
-(void)run
{
    AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
    if(appDelegate.runAll)
    {
        printf("\n*** running %s test %ld/%lu ...\n", [test.name UTF8String], (long)appDelegate.currentTest + 1,
               (unsigned long)[appDelegate.tests count]);
        printf("*** protocol: %s\n", [appDelegate.protocol UTF8String]);
        fflush(stdout);
    }
    if(server)
    {
        [self runServer];
    }
    else
    {
        [self runClient];
    }
}
-(void)add:(NSString*)s
{
    [viewController add:s];
}

-(void)clientComplete:(NSNumber*)rc
{
    if(clientHelper)
    {
        delete clientHelper;
        clientHelper = 0;
    }
    
    if([rc intValue] != 0)
    {
        [viewController add:[NSString stringWithFormat:@"client error: %@!\n", rc]];
        if(serverHelper)
        {
            serverHelper->shutdown();
        }
        ++error;
    }
    
    completed++;
    if(!server || completed == 2)
    {
        [viewController testRunComplete:error == 0];
    }
}

// Run in a separate thread.
-(void)runClient
{
    AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
    running++;
    
    std::string name = [[test name] UTF8String];
    std::string prefix = std::string("test_") + [[test prefix] UTF8String];
    std::string clt = [client UTF8String];
    
    TestConfig config;
    config.protocol = [[appDelegate protocol] UTF8String];
    config.type = collocated ?  TestConfigTypeColloc : TestConfigTypeClient;
    config.option = option;
    config.hasServer = [test hasServer];
    
    clientHelper = new MainHelperI(name, prefix + "/" + clt, config, self, @selector(add:), @selector(serverReady));
    clientHelper->run();
    int rc = clientHelper->status();
    [self performSelectorOnMainThread:@selector(clientComplete:) withObject:[NSNumber numberWithInt:rc] waitUntilDone:NO];
}

-(void)serverComplete:(NSNumber*)rc
{
    if(serverHelper)
    {
        delete serverHelper;
        serverHelper = 0;
    }
    
    if([rc intValue] != 0)
    {
        [viewController add:[NSString stringWithFormat:@"server error: %@!\n", rc]];
        ++error;
    }
    completed++;
    if(completed == 2)
    {
        [viewController testRunComplete:error == 0];
    }
}

// Run in a separate thread.
-(void)runServer
{
    AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
    running++;
    std::string name = [[test name] UTF8String];
    std::string prefix = std::string("test_") + [[test prefix] UTF8String];
    std::string srv = [server UTF8String];
    
    TestConfig config;
    config.protocol = [[appDelegate protocol] UTF8String];
    config.type = TestConfigTypeServer;
    config.option = option;
    config.hasServer = true;
    
    serverHelper = new MainHelperI(name, prefix + "/" + srv, config, self, @selector(add:), @selector(serverReady));
    serverHelper->run();
    int rc = serverHelper->status();
    [self performSelectorOnMainThread:@selector(serverComplete:) withObject:[NSNumber numberWithInt:rc] waitUntilDone:NO];
}

// Kick off the client.
-(void)serverReady
{
    [[viewController queue] addOperation:[[[NSInvocationOperation alloc]
                                           initWithTarget:self
                                           selector:@selector(runClient)
                                           object:nil] autorelease]];
}

@end
