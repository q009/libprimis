#include "engine.h"

/* input.h: SDL input handling
 *
 * SDL handles low-level window manager and key inputs for Imprimis
 * this file determines how the game parses the SDL information it is given
 *
 * includes handling for when the window manager tells the game to change state
 * and handling for how the game should react to key inputs (though obviously this
 * is rebindable by the client)
 *
 */
VARNP(relativemouse, userelativemouse, 0, 1, 1);

bool shouldgrab = false,
     grabinput = false,
     minimized = false,
     canrelativemouse = true,
     relativemouse = false;
int keyrepeatmask = 0,
    textinputmask = 0;
Uint32 textinputtime = 0;

VAR(textinputfilter, 0, 5, 1000);

void keyrepeat(bool on, int mask)
{
    if(on)
    {
        keyrepeatmask |= mask;
    }
    else
    {
        keyrepeatmask &= ~mask;
    }
}

void textinput(bool on, int mask)
{
    if(on)
    {
        if(!textinputmask)
        {
            SDL_StartTextInput();
            textinputtime = SDL_GetTicks();
        }
        textinputmask |= mask;
    }
    else
    {
        textinputmask &= ~mask;
        if(!textinputmask)
        {
            SDL_StopTextInput();
        }
    }
}

void inputgrab(bool on)
{
    if(on)
    {
        SDL_ShowCursor(SDL_FALSE);
        if(canrelativemouse && userelativemouse)
        {
            if(SDL_SetRelativeMouseMode(SDL_TRUE) >= 0)
            {
                SDL_SetWindowGrab(screen, SDL_TRUE);
                relativemouse = true;
            }
            else
            {
                SDL_SetWindowGrab(screen, SDL_FALSE);
                canrelativemouse = false;
                relativemouse = false;
            }
        }
    }
    else
    {
        SDL_ShowCursor(SDL_TRUE);
        if(relativemouse)
        {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            SDL_SetWindowGrab(screen, SDL_FALSE);
            relativemouse = false;
        }
    }
    shouldgrab = false;
}

vector<SDL_Event> events;

void pushevent(const SDL_Event &e)
{
    events.add(e);
}

static bool filterevent(const SDL_Event &event)
{
    switch(event.type)
    {
        case SDL_MOUSEMOTION:
            if(grabinput && !relativemouse && !(SDL_GetWindowFlags(screen) & SDL_WINDOW_FULLSCREEN))
            {
                if(event.motion.x == screenw / 2 && event.motion.y == screenh / 2)
                {
                    return false;  // ignore any motion events generated by SDL_WarpMouse
                }
            }
            break;
    }
    return true;
}

static inline bool pollevent(SDL_Event &event)
{
    while(SDL_PollEvent(&event))
    {
        if(filterevent(event))
        {
            return true;
        }
    }
    return false;
}

bool interceptkey(int sym)
{
    static int lastintercept = SDLK_UNKNOWN;
    int len = lastintercept == sym ? events.length() : 0;
    SDL_Event event;
    while(pollevent(event))
    {
        switch(event.type)
        {
            case SDL_MOUSEMOTION:
            {
                break;
            }
            default:
            {
                pushevent(event);
                break;
            }
        }
    }
    lastintercept = sym;
    if(sym != SDLK_UNKNOWN)
    {
        for(int i = len; i < events.length(); i++)
        {
            if(events[i].type == SDL_KEYDOWN && events[i].key.keysym.sym == sym)
            {
                events.remove(i);
                return true;
            }
        }
    }
    return false;
}

void ignoremousemotion()
{
    SDL_Event e;
    SDL_PumpEvents();
    //go through each event and do nothing
    while(SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION))
    {
        //(empty body)
    }
}

static void resetmousemotion()
{
    if(grabinput && !relativemouse && !(SDL_GetWindowFlags(screen) & SDL_WINDOW_FULLSCREEN))
    {
        SDL_WarpMouseInWindow(screen, screenw / 2, screenh / 2); //move to middle of screen
    }
}

static void checkmousemotion(int &dx, int &dy)
{
    for(int i = 0; i < events.length(); i++)
    {
        SDL_Event &event = events[i];
        if(event.type != SDL_MOUSEMOTION)
        {
            if(i > 0)
            {
                events.remove(0, i);
            }
            return;
        }
        dx += event.motion.xrel;
        dy += event.motion.yrel;
    }
    events.setsize(0);
    SDL_Event event;
    while(pollevent(event))
    {
        if(event.type != SDL_MOUSEMOTION)
        {
            events.add(event);
            return;
        }
        dx += event.motion.xrel;
        dy += event.motion.yrel;
    }
}

//handle different input types
void checkinput()
{
    SDL_Event event;
    bool mousemoved = false;
    while(events.length() || pollevent(event))
    {
        if(events.length())
        {
            event = events.remove(0);
        }
        switch(event.type)
        {
            case SDL_QUIT:
            {
                fatal("sdl quit");
                return;
            }
            case SDL_TEXTINPUT:
            {
                if(textinputmask && static_cast<int>(event.text.timestamp-textinputtime) >= textinputfilter)
                {
                    uchar buf[SDL_TEXTINPUTEVENT_TEXT_SIZE+1];
                    size_t len = decodeutf8(buf, sizeof(buf)-1, reinterpret_cast<const uchar *>(event.text.text), strlen(event.text.text));
                    if(len > 0)
                    {
                        buf[len] = '\0';
                        processtextinput((const char *)buf, len);
                    }
                }
                break;
            }
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                if(keyrepeatmask || !event.key.repeat)
                {
                    processkey(event.key.keysym.sym, event.key.state==SDL_PRESSED);
                }
                break;
            }
            case SDL_WINDOWEVENT:
            {
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_CLOSE:
                    {
                        fatal("window closed");
                        break;
                    }
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    {
                        shouldgrab = true;
                        break;
                    }
                    case SDL_WINDOWEVENT_ENTER:
                    {
                        inputgrab(grabinput = true);
                        break;
                    }
                    case SDL_WINDOWEVENT_LEAVE:
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                    {
                        inputgrab(grabinput = false);
                        break;
                    }
                    case SDL_WINDOWEVENT_MINIMIZED:
                    {
                        minimized = true;
                        break;
                    }
                    case SDL_WINDOWEVENT_MAXIMIZED:
                    case SDL_WINDOWEVENT_RESTORED:
                    {
                        minimized = false;
                        break;
                    }
                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        break;
                    }
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        SDL_GetWindowSize(screen, &screenw, &screenh);
                        if(!(SDL_GetWindowFlags(screen) & SDL_WINDOW_FULLSCREEN))
                        {
                            //need to cast enums to ints for std's clamp implementation
                            scr_w = std::clamp(screenw, static_cast<int>(SCR_MINW), static_cast<int>(SCR_MAXW));
                            scr_h = std::clamp(screenh, static_cast<int>(SCR_MINH), static_cast<int>(SCR_MAXH));
                        }
                        gl_resize();
                        break;
                    }
                }
                break;
            }
            case SDL_MOUSEMOTION:
            {
                if(grabinput)
                {
                    int dx = event.motion.xrel,
                        dy = event.motion.yrel;
                    checkmousemotion(dx, dy);
                    if(!UI::movecursor(dx, dy))
                    {
                        mousemove(dx, dy);
                    }
                    mousemoved = true;
                }
                else if(shouldgrab)
                {
                    inputgrab(grabinput = true);
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                switch(event.button.button)
                {
                    case SDL_BUTTON_LEFT:
                    {
                        processkey(-1, event.button.state==SDL_PRESSED);
                        break;
                    }
                    case SDL_BUTTON_MIDDLE:
                    {
                        processkey(-2, event.button.state==SDL_PRESSED);
                        break;
                    }
                    case SDL_BUTTON_RIGHT:
                    {
                        processkey(-3, event.button.state==SDL_PRESSED);
                        break;
                    }
                    case SDL_BUTTON_X1:
                    {
                        processkey(-6, event.button.state==SDL_PRESSED);
                        break;
                    }
                    case SDL_BUTTON_X2:
                    {
                        processkey(-7, event.button.state==SDL_PRESSED);
                        break;
                    }
                }
                break;
            }
            case SDL_MOUSEWHEEL:
            {
                //up
                if(event.wheel.y > 0)
                {
                    processkey(-4, true);
                    processkey(-4, false);
                }
                //down
                else if(event.wheel.y < 0)
                {
                    processkey(-5, true);
                    processkey(-5, false);
                }
                break;
            }
        }
    }
    if(mousemoved)
    {
        resetmousemotion();
    }
}
