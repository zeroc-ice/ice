// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <TestSelectController.h>
#import <Test.h>
#import <AppDelegate.h>
#import <TestViewController.h>

@interface TestSelectController()

@property (nonatomic, retain) NSArray* tests;
@property (nonatomic, retain) UIPickerView* pickerView;
@property (nonatomic, retain) UISwitch* sslSwitch;
@property (nonatomic, retain) UISegmentedControl* protocol;
@property (nonatomic, retain) TestViewController* testViewController;

@end

@implementation TestSelectController

@synthesize tests;
@synthesize pickerView;
@synthesize protocol;
@synthesize testViewController;

- (void)viewDidLoad
{
    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    self.tests = appDelegate.tests;
    [super viewDidLoad];
}

-(void)viewWillAppear:(BOOL)animated
{
    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    [pickerView selectRow:appDelegate.currentTest inComponent:0 animated:NO];
    for (int i = 0; i != [self.protocol numberOfSegments]; ++i)
    {
        if([[self.protocol titleForSegmentAtIndex:i] isEqualToString:[appDelegate.protocol uppercaseString]])
        {
            self.protocol.selectedSegmentIndex = i;
            break;
        }
    }

    if(appDelegate.runAll)
    {
        [self.navigationController pushViewController:self.testViewController animated:YES];
    }
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
    [tests release];
    [pickerView release];
    [protocol release];
    [loopSwitch release];
    [testViewController release];

    [super dealloc];
}
#endif

-(TestViewController*)testViewController
{
    // Instantiate the test view controller if necessary.
    if (testViewController == nil)
    {
        NSString* nib;
        if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        {
            nib = @"TestView-iPad";
        }
        else
        {
            nib = @"TestView";
        }
        testViewController = [[TestViewController alloc] initWithNibName:nib bundle:nil];
    }
    return testViewController;
}

#pragma mark -

-(IBAction)runTest:(id)sender
{
    NSUInteger row = [pickerView selectedRowInComponent:0];
    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    appDelegate.currentTest = row;
    appDelegate.protocol = [[protocol titleForSegmentAtIndex:[protocol selectedSegmentIndex]] lowercaseString];
    appDelegate.loop = loopSwitch.isOn;

    TestViewController* controller = self.testViewController;
    [self.navigationController pushViewController:controller animated:YES];
}

#pragma mark UIPickerViewDelegate

- (NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
    Test* test = (Test*)[tests objectAtIndex:row];
    return test.name;
}

#pragma mark UIPickerViewDataSource

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
    return 1;
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)componen
{
    return tests.count;
}
@end
