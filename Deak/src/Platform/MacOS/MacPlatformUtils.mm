#import "Deak/Core/Application.h"
#import "MacPlatformUtils.h"

#import <Cocoa/Cocoa.h>
#import <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#import <GLFW/glfw3native.h>

const char* OpenFileUsingCocoa(const char* filter)
{
    NSWindow* window = (NSWindow*)glfwGetCocoaWindow((GLFWwindow*)Deak::Application::Get().GetWindow().GetNativeWindow());

    if (!window)
    {
        return nullptr;
    }

        NSOpenPanel* openPanel = [NSOpenPanel openPanel];
        [openPanel setAllowsMultipleSelection:NO];
        [openPanel setCanChooseDirectories:NO];
        [openPanel setCanChooseFiles:YES];

        NSString* filterString = [NSString stringWithUTF8String:filter];
        NSArray* fileTypes = @[filterString];
        [openPanel setAllowedFileTypes:fileTypes];

        if ([openPanel runModal] == NSModalResponseOK)
        {
            NSString* filePath = [[openPanel URLs]objectAtIndex:0].path;
            return strdup([filePath UTF8String]);
        }

        return nullptr;
}

const char* SaveFileUsingCocoa(const char* filter)
{
    NSWindow* window = (NSWindow*)glfwGetCocoaWindow((GLFWwindow*)Deak::Application::Get().GetWindow().GetNativeWindow());

    if (!window)
    {
        return nullptr;
    }

        NSSavePanel* savePanel = [NSSavePanel savePanel];

        NSString* filterString = [NSString stringWithUTF8String:filter];
        NSArray* fileTypes = @[filterString];
        [savePanel setAllowedFileTypes:fileTypes];
        [savePanel setCanCreateDirectories:YES];
        [savePanel setShowsTagField:NO];

        if ([savePanel runModal] == NSModalResponseOK)
        {
            NSURL* selectedFileURL = [savePanel URL];
            NSString* filePath = [selectedFileURL path];

            // Check if file exists and confirm overwrite
            if ([[NSFileManager defaultManager] fileExistsAtPath:filePath]) 
            {
                NSAlert *alert = [[NSAlert alloc] init];
                [alert setMessageText:@"Confirm Overwrite"];
                [alert setInformativeText:@"A file with the same name already exists. Do you want to replace it?"];
                [alert addButtonWithTitle:@"Yes"];
                [alert addButtonWithTitle:@"No"];

                if ([alert runModal] == NSAlertFirstButtonReturn) // User confirmed overwrite
                {
                    return strdup([filePath UTF8String]);
                }
                else // User cancelled overwrite
                {
                    NSLog(@"User cancelled overwrite.");
                    return nullptr;
                }
            }
            else 
            {
                return strdup([filePath UTF8String]);
            }
        }
        else 
        {
            NSLog(@"Save operation was cancelled or no file path provided.");
        }

        return nullptr;
}

int PromptSaveOnExitUsingCocoa()
{
     NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Do you want to save changes before exiting?"];
    [alert setInformativeText:@"Your changes will be lost if you don't save them."];
    [alert addButtonWithTitle:@"Save"];        // First button
    [alert addButtonWithTitle:@"Don't Save"];  // Second button
    [alert addButtonWithTitle:@"Cancel"];      // Third button

    NSInteger result = [alert runModal];

    switch (result) {
        case NSAlertFirstButtonReturn: return 0;  // Corresponding to 'Save'
        case NSAlertSecondButtonReturn: return 1;  // Corresponding to 'Don't Save'
        case NSAlertThirdButtonReturn: return 2;  // Corresponding to 'Cancel'
        default: return 2;  // Defaulting to 'Cancel'
    }
}


