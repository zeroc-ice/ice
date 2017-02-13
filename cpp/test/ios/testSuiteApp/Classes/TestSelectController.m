// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <TestSelectController.h>
#import <TestUtil.h>
#import <AppDelegate.h>
#import <TestViewController.h>

@interface TestSelectController()

@property (nonatomic, retain) NSArray* testSuites;
@property (nonatomic, retain) UIPickerView* pickerView;
@property (nonatomic, retain) UISegmentedControl* protocol;
@property (nonatomic, retain) TestViewController* testViewController;

@end

@implementation TestSelectController

@synthesize testSuites;
@synthesize pickerView;
@synthesize protocol;
@synthesize testViewController;

- (void)viewDidLoad
{
    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    self.testSuites = appDelegate.testSuites;
    [super viewDidLoad];
}

-(void)viewWillAppear:(BOOL)animated
{
    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    [pickerView selectRow:appDelegate.currentTestSuite inComponent:0 animated:NO];
    for (int i = 0; i != [self.protocol numberOfSegments]; ++i)
    {
        if([[self.protocol titleForSegmentAtIndex:i] isEqualToString:[appDelegate.protocol uppercaseString]])
        {
            self.protocol.selectedSegmentIndex = i;
            break;
        }
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

- (void)dealloc
{
    [testSuites release];
    [pickerView release];
    [protocol release];
    [loopSwitch release];
    [testViewController release];

    [super dealloc];
}

-(TestViewController*)testViewController
{
    // Instantiate the test view controller if necessary.
    if(testViewController == nil)
    {
		NSString* nib;
		if(UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
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
    NSInteger row = [pickerView selectedRowInComponent:0];
    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    appDelegate.currentTestSuite = row;
    appDelegate.protocol = [[protocol titleForSegmentAtIndex:[protocol selectedSegmentIndex]] lowercaseString];
    appDelegate.loop = loopSwitch.isOn;
    [self.navigationController pushViewController:self.testViewController animated:YES];
}

#pragma mark UIPickerViewDelegate

- (NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
    TestSuite* test = (TestSuite*)[testSuites objectAtIndex:row];
    return test.testSuiteId;
}

#pragma mark UIPickerViewDataSource

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
    return 1;
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)componen
{
    return testSuites.count;
}
@end
