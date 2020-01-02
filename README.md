# Naboris
Simple, fast, minimalist web framework for [OCaml](https://ocaml.org)/[ReasonML](https://reasonml.github.io) built on [httpaf](https://github.com/inhabitedtype/httpaf) and [lwt](https://github.com/ocsigen/lwt).

[![Build Status](https://travis-ci.com/shawn-mcginty/naboris.svg?branch=master)](https://travis-ci.com/shawn-mcginty/naboris)
[![opam version 0.0.7](https://img.shields.io/static/v1?label=opam&message=0.0.7&color=E7C162)](https://opam.ocaml.org/packages/naboris/)

```ocaml
let serverConfig: Naboris.ServerConfig.t(unit) = Naboris.ServerConfig.create()
  |> Naboris.ServerConfig.setRequestHandler((route, req, res) => switch(route.path) {
    | ["hello"] =>
      res
        |> Naboris.Res.status(200)
        |> Naboris.Res.text(req, "Hello world!");
      Lwt.return_unit;
    | _ =>
      res
        |> Naboris.Res.status(404)
        |> Naboris.Res.text(req, "Resource not found.");
      Lwt.return_unit;
  });

Naboris.listenAndWaitForever(3000, serverConfig);
/* In a browser navigate to http://localhost:3000/hello */
```

## Contents
* [Getting Started](#getting-started)
    * [Installation](#installation)
    * [Server Config](#server-config)
    * [Routing](#routing)
    * [Session Data](#session-data)
* [Advanced](#advanced)
	* [Middlewares](#middlewares)
* [Development](#development)

```
                                                           
 @@@@@  @@@@  @@@@@                                        
 *@*   @@@@@@   @@&                                        
  @@&  .@@@@  @@@/        @@,         (@@@                 
    ,    @@             @@@@@@@      @@@@@@@               
                       @@@@@@@@,    @@@@@@@@@              
        @@@*           @@@@@@@@@@@@@@@@@@@@@@              
       &@@@@          @@@@@@@@@@@@@.      &@@              
    @@@@@@@@           @@@@@@@@@@@@@@#(%(                  
    @@@@@  @@         .@@@@@@@@@@@@@@@@@@@@@*              
       ,@#  @*       @@@@@@@@@@@@@@@@@@@@@@@@@             
     # ,@@   @@     ,@@@@@@@@@@@@@@@@@@@@@@@@@@            
    .@@@@@.  .@@@   @@@@@@@@@@@@@@@@@@@@@@@@@@@@           
         @@.   @@@@ %@@@@@@@@@@@@@@@@@@@@@@@@@@@&          
         &@@*       .@@@.@@@@@@@@@@@*     (@@@@@@@@@@@@@@  
       @@@@@@@       @   @@@@@@@@@@   %@&   @@@@@@@@@@@*   
        @@  @@@@@      .@@@@@@@@ @  /@@@@@@     #@@@@@     
             @@@@@@    @@@@@@@@@    @@@@@@@@       @       
            @@@   %@   @@  , .@@   %@@(@&,@@%              
              ,          @@@@*       @@@@@                 
                         @@@@@        @@@@                 
                         @@@@@        @@@.                 
                         @@@@@        @@@%                 
                         @@@@@        @@@                  
                          %@           @.                  
                          (@           @                   
                          .%           ,                   
                                                           
                          @@(          @@                  
```

## Getting Started

### Installation

#### opam
```bash
opam install naboris
```

#### esy
```json
	"@opam/naboris": "^0.0.7"
```

#### dune
```
(libraries naboris)
```

### Server Config
The `Naboris.ServerConfig.t('sessionData)` type will define the way your server will handle requests.

#### Creating a Server Config

There are a number of helper functions for building server config records.

##### ServerConfig.create
__create__ is used to generate a default server config object, this will be the starting point.
```ocaml
let create: unit => ServerConfig.t('sessionData);
```

#### ServerConfig.setOnListen
__setOnListen__ will set the function that will be called once the server has started and is listening for connections.  The `onListen` function has the type signature `unit => unit`.
```ocaml
let setOnListen: (unit => unit, ServerConfig.t('sessionData)) => ServerConfig.t('sessionData)
```

#### ServerConfig.setRequestHandler
__setRequestHandler__ will set the main request handler function on the config.  This function is the main entry point for http requests and usually where routing the request happens.  The `requestHandler` function has the type signature `(Route.t, Req.t('sessionData), Res.t) => Lwt.t(unit)`.
```ocaml
let setRequestHandler: ((Route.t, Req.t('sessionData), Res.t) => Lwt.t(unit), ServerConfig.t('sessionData)) => ServerConfig.t('sessionData)
```

### Routing
Routin is intended to be done via pattern matching in the main `requestHandler` function.  This function takes as it's first argument a `Route.t` record, which looks like this:
```ocaml
/* module Naboris.Route */
type t = {
  path: list(string),
  meth: Method.t,
  rawQuery: string,
  query: Query.QueryMap.t(list(string)),
};
```

For these examples we'll be matching on `path` and `meth`.
> Note: `path` is the url path broken into an array by the `/` separators.
> e.g. `/user/1/contacts` would look like this: `["user", "1", "contacts"]`

```ocaml
let requestHandler = (route, req, res) => switch (route.meth, route.path) {
  | (Naboris.Method.GET, ["user", userId, "contacts"]) =>
    /* Use pattern matching to pull parameters out of the url */
    let contacts = getContactsByUserId(userId);
    let contactsJsonString = serializeToJson(contacts);
    res
      |> Naboris.Res.status(200)
      |> Naboris.Res.json(req, contactsJsonString);
    Lwt.return_unit;
  | (Naboris.Method.PUT, ["user", userId, "contacts"]) =>
    /* for the sake of this example we're not using ppx or infix */
    /* lwt promises can be made much easier to read by using these */
    Lwt.bind(
      Naboris.Req.getBody(req),
      bodyStr => {
      	let newContacts = parseJsonString(bodyStr);
        addNewContactsToUser(userId, newContacts);
        res
          |> Naboris.Res.status(201)
          |> Naboris.Res.text(req, "Created");
        Lwt.return_unit;
      },
    )
  | _ =>
      res
        |> Naboris.Res.status(404)
        |> Naboris.Res.text(req, "Resource not found.");
      Lwt.return_unit;
};
```

### Session Data
Many `Naboris` types take the parameter `'sessionData` this represents a custom data type that will define session data that will be attached to an incoming request.

#### sessionGetter
__Naboris.ServerConfig.setSessionGetter__ will set the configuration with a function with the signature `option(string) => Lwt.t(option(Naboris.Session.t('sessionData)))`.  That's a complicated type signature that expresses that the request may or may not have a `sessionId`; and given that fact it may or may not return a session.
```ocaml
type userData = {
  userId: int,
  username: string,
  firstName: string,
  lastName: string,
};

let serverConfig: Naboris.ServerConfig(userData) = Naboris.ServerConfig.create()
  |> setSessionGetter(sessionId => switch(sessionId) {
    | Some(id) =>
      /* for the sake of this example we're not using ppx or infix */
      /* lwt promises can be made much easier to read by using these */
      Lwt.bind(getUserDataById(id),
        userData => {
          let session = Naboris.Session.create(id, userData);
          Lwt.return(Some(session));
        }
      );
    | None => Lwt.return(None);
  })
  |> setRequestHandler((route, req, res) => switch(route.meth, route.path) {
    | (Naboris.Method.POST, ["login"]) =>
      let (req2, res2, _sessionId) =
        /* Begin a session */
        Naboris.SessionManager.startSession(
          req,
          res,
          {
            userId: 1,
            username: "foo",
            firstName: "foo",
            lastName: "bar",
          },
        );
        Naboris.Res.status(200, res2) |> Naboris.Res.text(req2, "OK");
        Lwt.return_unit;
    | (Naboris.Method.GET, ["who-am-i"]) =>
      /* Get session data from the request */
      switch (Naboris.Req.getSessionData(req)) {
      | None =>
        Naboris.Res.status(404, res) |> Naboris.Res.text(req, "Not found")
      | Some(userData) =>
        Naboris.Res.status(200, res)
        |> Naboris.Res.text(req, userData.username)
      };
      Lwt.return_unit;
  });
```

## Advanced

### Middlewares
More info coming soon...

## Development
Any help would be greatly appreciated! 👍

### To run tests

```bash
esy install
npm run test
```