<?php
   
class Route 
{
    public function Lite($routes){

         $dispatcher = \FastRoute\simpleDispatcher(function(FastRoute\RouteCollector $r)use($routes) {
            foreach ($routes as $route){
                $r->addRoute($route[0], $route[1], $route[2]);
            }
        });
        
        // Fetch method and URI from somewhere
        $httpMethod = $_SERVER['REQUEST_METHOD'];

        $uri = $_SERVER['REQUEST_URI'];

        $handler=null;
        // Strip query string (?foo=bar) and decode URI
        if (false !== $pos = strpos($uri, '?')) {
            $uri = substr($uri, 0, $pos);
        }
        $uri = rawurldecode($uri);
        
        $routeInfo = $dispatcher->dispatch($httpMethod, $uri);
        
        switch ($routeInfo[0]) {
            case FastRoute\Dispatcher::NOT_FOUND:
                // ... 404 Not Found
                break;
            case FastRoute\Dispatcher::METHOD_NOT_ALLOWED:
                $allowedMethods = $routeInfo[1];
                // ... 405 Method Not Allowed
                break;
            case FastRoute\Dispatcher::FOUND:
                $handler = $routeInfo[1];
                $vars = $routeInfo[2];
              
                // ... call $handler with $vars
                break;
        }

        
        return $handler;

    }

}





?>