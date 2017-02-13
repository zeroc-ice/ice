// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <TestViewController.h>
#import <TestUtil.h>
#import <AppDelegate.h>

NSArray* noSSL = @[ @"Ice/metrics" ];
NSArray* noWS = @[ @"Ice/metrics" ];
NSArray* noCpp11 = @[ @"Ice/custom" ];

static NSArray* protocols = @[ @"tcp", @"ssl", @"ws", @"wss" ];
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
@property (retain) TestSuite* testSuite;

-(void)add:(NSString*)d;
-(void)startTest;
@end

@interface TestRun : NSObject
{
    NSString* testSuiteId;
    TestCase* testCase;
    NSString* protocol;
    int completed;
    int running;
    int error;
    MainHelperI* clientHelper;
    MainHelperI* serverHelper;
    TestViewController* viewController;
}

+(id) testRun:(NSString*)testSuiteId testCase:(TestCase*)testCase protocol:(NSString*)protocol;

-(NSInvocationOperation*) runInvocation:(TestViewController*)callback;
@end

@implementation TestViewController

@synthesize output;
@synthesize activity;
@synthesize nextButton;
@synthesize currentMessage;
@synthesize messages;
@synthesize queue;
@synthesize testSuite;

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
    self.testSuite = (TestSuite*)[appDelegate.testSuites objectAtIndex:appDelegate.currentTestSuite];
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
    [testSuite release];

    [super dealloc];
}

#pragma mark -

-(void)startTest
{
    self.title = testSuite.testSuiteId;
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
#ifdef ICE_CPP11_MAPPING
    if([testSuite isIn:noCpp11])
    {
        [self add:@"C++11 not supported by this test\n"];
        [self testComplete:YES];
        return;
    }
#endif
    if(([appDelegate.protocol isEqualToString:@"ws"] || [appDelegate.protocol isEqualToString:@"wss"]) &&
       [testSuite isIn:noWS])
    {
        [self add:@"WS not supported by this test\n"];
        [self testComplete:YES];
        return;
    }
    if([appDelegate.protocol isEqualToString:@"ssl"] &&
       [testSuite isIn:noSSL])
    {
        [self add:@"SSL not supported by this test\n"];
        [self testComplete:YES];
        return;
    }

    NSMutableArray* testRuns = [NSMutableArray array];
    for(id testCase in testSuite.testCases)
    {
        [testRuns addObject:[TestRun testRun:testSuite.testSuiteId testCase:testCase protocol:appDelegate.protocol]];
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
    NSInteger next = (appDelegate.currentTestSuite + 1) % (appDelegate.testSuites.count);
    TestSuite* nextTestSuite = (TestSuite*)[appDelegate.testSuites objectAtIndex:next];
    NSString* buttonTitle = [NSString stringWithFormat:@"Run %@", nextTestSuite.testSuiteId];
    [nextButton setTitle:buttonTitle forState:UIControlStateNormal];
    self.testSuite = nil;
    [testRunEnumator release];
    testRunEnumator = nil;

    if([appDelegate testCompleted:success])
    {
        NSAssert(self.testSuite == nil, @"testSuite == nil");
        self.testSuite = (TestSuite*)[appDelegate.testSuites objectAtIndex:appDelegate.currentTestSuite];
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
        NSAssert(self.testSuite == nil, @"testSuite == nil");
        AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
        self.testSuite = (TestSuite*)[appDelegate.testSuites objectAtIndex:appDelegate.currentTestSuite];
        [self startTest];
    }
}
-(NSOperationQueue*) queue
{
    return queue;
}
-(void)add:(NSString*)s
{
    [currentMessage appendString:s];
    NSRange range = [currentMessage rangeOfString:@"\n" options:NSBackwardsSearch];
    if(range.location != NSNotFound)
    {
        [messages addObject:[[currentMessage copy] autorelease]];
        [currentMessage deleteCharactersInRange:NSMakeRange(0, currentMessage.length)];

        // reloadData hangs if called too rapidly... we call at most every 100ms.
        if(!reloadScheduled)
        {
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
-(id) init:(NSString*)ts testCase:(TestCase*)tc protocol:(NSString*)p
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->testSuiteId = ts;
    self->testCase = tc;
    self->protocol = p;
    self->completed = 0;
    self->error = 0;
    self->running = 0;
    return self;
}
+(id) testRun:(NSString*)ts testCase:(TestCase*)tc protocol:(NSString*)p
{
    return [[[TestRun alloc] init:ts testCase:tc protocol:p] autorelease];
}
-(NSInvocationOperation*) runInvocation:(TestViewController*)ctl
{
    viewController = ctl;
    return [[[NSInvocationOperation alloc] initWithTarget:self selector:@selector(run) object:nil] autorelease];
}
-(void)run
{
//    [self add:[NSString stringWithFormat:@"[ running %@ ]\n", testCase.name]];
    if(testCase.server)
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
    if(!testCase.server || completed == 2)
    {
        [viewController testRunComplete:error == 0];
    }
}

// Run in a separate thread.
-(void)runClient
{
    running++;

    std::string name = [testCase.name UTF8String];
    std::string prefix = std::string("test/") + [testSuiteId UTF8String];
    replace(prefix.begin(), prefix.end(), '/', '_');
    std::string clt = [testCase.client UTF8String];

    TestConfig config;
    config.protocol = [protocol UTF8String];
    for(id arg in testCase.args)
    {
        config.args.push_back([arg UTF8String]);
    }

    clientHelper = new MainHelperI(name, prefix + "/" + clt, TestTypeClient, config,
                                   self, @selector(add:), @selector(serverReady));
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
    running++;

    std::string name = [testCase.name UTF8String];
    std::string prefix = std::string("test/") + [testSuiteId UTF8String];
    replace(prefix.begin(), prefix.end(), '/', '_');
    std::string srv = [testCase.server UTF8String];

    TestConfig config;
    config.protocol = [protocol UTF8String];
    for(id arg in testCase.args)
    {
        config.args.push_back([arg UTF8String]);
    }

    serverHelper = new MainHelperI(name, prefix + "/" + srv, TestTypeServer, config,
                                   self, @selector(add:), @selector(serverReady));
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
