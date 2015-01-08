// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <Parser.h>
#import <string.h>
#import <stdio.h>

extern FILE* yyin;
Parser* parser;

@interface Parser()

@property(nonatomic, retain) DemoBookQueryResultPrx* query;
@property(nonatomic, retain) DemoBookDescription* current;
@property(nonatomic, retain) id<DemoLibraryPrx> library;

@end

@implementation Parser

@synthesize query;
@synthesize current;
@synthesize library;

-(id)initWithLibrary:(id<DemoLibraryPrx>)lib
{
    if((self = [super init]))
    {
        self.library = lib;
    }
    return self;
}

+(id)parserWithLibrary:(id<DemoLibraryPrx>)library
{
    return [[Parser alloc] initWithLibrary:library];
}

-(void)usage
{
    printf("help                    Print this message.\n"
           "exit, quit              Exit this program.\n"
           "add isbn title authors  Create new book.\n"
           "isbn NUMBER             Find all books that start with the given ISBN number.\n"
           "authors NAME            Find all books by the given authors.\n"
           "title NAME              Find all books which have the given title.\n"
           "next                    Set the current book to the next one that was found.\n"
           "current                 Display the current book.\n"
           "rent NAME               Rent the current book for customer NAME.\n"
           "return                  Return the currently rented book.\n"
           "remove                  Permanently remove the current book from the library.\n");
}

-(void)addBook:(NSArray*)args
{
    if(args.count != 3)
    {
        [self errorWithString:@"`add' requires exactly three arguments (type `help' for more info)"];
        return;
    }

    @try
    {
        NSString* isbn = [args objectAtIndex:0];
        NSString* title = [args objectAtIndex:1];

        NSArray* authors = [[args objectAtIndex:2] componentsSeparatedByString:@","];
        [library createBook:isbn title:title authors:authors];
        printf("added new book with isbn %s\n", [isbn UTF8String]);
    }
    @catch(DemoBookExistsException* ex)
    {
        [self errorWithString:@"the book already exists"];
    }
    @catch(ICELocalException* ex)
    {
        [self errorWithString:[ex description]];
    }
}
-(void)findIsbn:(NSArray*)args
{
    if(args.count != 1)
    {
        [self errorWithString:@"`isbn' requires exactly one argument (type `help' for more info)"];
        return;
    }

    @try
    {
        if(query != nil)
        {
            @try
            {
                [query destroy];
            }
            @catch(ICEException* e)
            {
                // Ignore
            }

            self.query =  nil;
            self.current = nil;
        }

        id<DemoBookQueryResultPrx> q;
	DemoMutableBookDescriptionSeq *results;
	int nrows;
        [library queryByIsbn:[args objectAtIndex:0] n:1 first:&results nrows:&nrows result:&q];

	printf("%d results\n", nrows);
	if(nrows == 0)
	{
            return;
	}

        self.current = [results objectAtIndex:0];
        self.query = q;
        
        [self printCurrent];
    }
    @catch(ICELocalException* ex)
    {
        [self errorWithString:[ex description]];
    }
}

-(void)findAuthors:(NSArray*)args
{
    if(args.count != 1)
    {
        [self errorWithString:@"`authors' requires exactly one argument (type `help' for more info)"];
        return;
    }

    @try
    {
        if(query != nil)
        {
            @try
            {
                [query destroy];
            }
            @catch(ICEException* e)
            {
                // Ignore
            }
            self.query = nil;
            self.current = nil;
        }

        id<DemoBookQueryResultPrx> q;
	DemoMutableBookDescriptionSeq *results;
	int nrows;
        [library queryByAuthor:[args objectAtIndex:0] n:1 first:&results nrows:&nrows result:&q];

	printf("%d results\n", nrows);
	if(nrows == 0)
	{
            return;
	}

        self.current = [results objectAtIndex:0];
        self.query = q;

        [self printCurrent];
    }
    @catch(ICELocalException* ex)
    {
        [self errorWithString:[ex description]];
    }
}

-(void)findTitle:(NSArray*)args
{
    if(args.count != 1)
    {
        [self errorWithString:@"`title' requires exactly one argument (type `help' for more info)"];
        return;
    }

    @try
    {
        if(query != nil)
        {
            @try
            {
                [query destroy];
            }
            @catch(ICEException* e)
            {
                // Ignore
            }
            self.query = nil;
            self.current = nil;
        }

        id<DemoBookQueryResultPrx> q;
	DemoMutableBookDescriptionSeq *results;
	int nrows;
        [library queryByTitle:[args objectAtIndex:0] n:1 first:&results nrows:&nrows result:&q];

	printf("%d results\n", nrows);
	if(nrows == 0)
	{
            return;
	}

        self.current = [results objectAtIndex:0];
        self.query = q;
        [self printCurrent];
    }
    @catch(ICELocalException* ex)
    {
        [self errorWithString:[ex description]];
    }
}

-(void)nextFoundBook
{
    if(query == nil)
    {
        printf("no next book\n");
        return;
    }

    @try
    {
        BOOL destroyed;
        NSArray* next = [query next:1 destroyed:&destroyed];
        if(next.count > 0)
        {
            self.current = [next objectAtIndex:0];
        }
        else
        {
            NSAssert(destroyed, @"");
            self.current = nil;
        }

        if(destroyed)
        {
            self.query = nil;
        }

        [self printCurrent];
    }
    @catch(ICEObjectNotExistException* ex)
    {
        printf("the query object no longer exists\n");
    }
    @catch(ICELocalException* ex)
    {
        [self errorWithString:[ex description]];
    }
}

-(void)printCurrent
{
    if(current != nil)
    {
        printf("current book is:\n");
        printf("isbn: %s\n", [current.isbn UTF8String]);
        printf("title: %s\n", [current.title UTF8String]);
        NSMutableString* auth = [[NSMutableString alloc] init];
        for(NSString* s in current.authors)
        {
            if(s != [current.authors objectAtIndex:0])
            {
                [auth appendString:@", "];
            }
            [auth appendString:s];
        }
        printf("authors: %s\n",  [auth UTF8String]);
        if([current.rentedBy length] > 0)
        {
            printf("rented: %s\n", [current.rentedBy UTF8String]);
        }
    }
    else
    {
        printf("no current book\n");
    }
}

-(void)rentCurrent:(NSArray*)args
{
    if(args.count != 1)
    {
        [self errorWithString:@"`rent' requires exactly one argument (type `help' for more info)"];
        return;
    }

    @try
    {
        if(current != nil)
        {
            [current.proxy rentBook:[args objectAtIndex:0]];
            printf("the book is now rented by `%s'\n", [[args objectAtIndex:0] UTF8String]);
            self.current  = [current.proxy describe];
        }
        else
        {
            printf("no current book\n");
        }
    }
    @catch(DemoBookRentedException* ex)
    {
        printf("the book has already been rented.\n");
    }
    @catch(ICEObjectNotExistException* ex)
    {
        printf("current book no longer exists\n");
    }
    @catch(ICELocalException* ex)
    {
        [self errorWithString:[ex description]];
    }
}

-(void)returnCurrent
{
    @try
    {
        if(current != nil)
        {
            [current.proxy returnBook];
            printf( "the book has been returned.\n");
            self.current = [current.proxy describe];
        }
        else
        {
            printf("no current book\n");
        }
    }
    @catch(DemoBookNotRentedException* ex)
    {
        printf("the book is not currently rented.\n");
    }
    @catch(ICEObjectNotExistException* ex)
    {
        printf("current book no longer exists\n");
    }
    @catch(ICELocalException* ex)
    {
        [self errorWithString:[ex description]];
    }
}

-(void)removeCurrent
{
    @try
    {
        if(current != nil)
        {
            [current.proxy destroy];
            self.current = nil;
            printf("removed current book\n");
        }
        else
        {
            printf("no current book\n");
        }
    }
    @catch(ICEObjectNotExistException* ex)
    {
        printf("current book no longer exists\n");
    }
    @catch(ICELocalException* ex)
    {
        [self errorWithString:[ex description]];
    }
}

-(const char*)getPrompt
{
    if(cont)
    {
        cont = NO;
        return "(cont) ";
    }
    else
    {
        return ">>> ";
    }
}

-(int)getInput:(char*)buf max:(int)max
{
    printf("%s", self.getPrompt);
    fflush(stdout);

    NSMutableData* line = [[ NSMutableData alloc] init];
    while(true)
    {
        char c = (char)(getc(yyin));
        if(c == EOF)
        {
            if([line length])
            {
                c = '\n';
                [line appendBytes:&c length:1];
            }
            break;
        }

        [line appendBytes:&c length:1];

        if(c == '\n')
        {
            break;
        }
    }
    NSString* ss = [[NSString alloc] initWithData:line encoding:NSUTF8StringEncoding];
    int len;
    const char* utf8 = [ss UTF8String];
    
    len = strlen(utf8);
    if(len > max)
    {
        [self errorWithString:@"input line too long"];
        buf[0] = EOF;
        return 1;
    }
    else
    {
        strcpy(buf, utf8);
    }

    return len;
}

-(void)continueLine
{
    cont = YES;
}

-(void)error:(const char*)s
{
    fprintf(stderr, "error: %s\n", s);
}

-(void)errorWithString:(NSString*)s
{
    fprintf(stderr, "error: %s\n", [s UTF8String]);
}

-(void)warning:(const char*)s
{
    fprintf(stderr, "warning: %s\n", s);
}

-(void)warningWithString:(NSString*)s
{
    fprintf(stderr, "warning: %s\n", [s UTF8String]);
}

-(int)parse
{
    NSAssert(!parser, @"");
    parser = self;

    errors = 0;
    yyin = stdin;
    NSAssert(yyin, @"");

    cont = false;

    self.query = nil;
    self.current = nil;

    int status = yyparse();
    if(errors)
    {
        status = EXIT_FAILURE;
    }

    parser = nil;
    return status;
}
@end
