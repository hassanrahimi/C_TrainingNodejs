
import express, { Express, Request, Response } from "express";
import dotenv from "dotenv";
import ffi from 'ffi-napi';
import ref from 'ref-napi';
dotenv.config();

const app: Express = express();
const port = process.env.PORT || 3000;

app.get("/", (req: Request, res: Response) => {
    res.send("Express + TypeScript Server");
    exampleLib.increment_counter(10);
    console.log('After incrementing by 10:', exampleLib.get_counter());
});

// Load the shared library
const exampleLib = ffi.Library('./example', {
    'get_counter': ['int', []],
    'increment_counter': ['void', ['int']],
    'reset_counter': ['void', []]
});

// Call the C functions and demonstrate global state manipulation
console.log('Initial counter value:', exampleLib.get_counter());

exampleLib.increment_counter(10);
console.log('After incrementing by 10:', exampleLib.get_counter());

exampleLib.increment_counter(5);
console.log('After incrementing by 5:', exampleLib.get_counter());

exampleLib.reset_counter();
console.log('After resetting counter:', exampleLib.get_counter());

app.get("/createpipeline", (req: Request, res: Response) => {
    res.send("Express + TypeScript Server");
    const mylibCfunctionCreatePipeline = ffi.Library('./mixing',{ 'createPipline': ['int', ['string']]});
    mylibCfunctionCreatePipeline.createPipline("testtttttt");
});



app.listen(port, () => {
    console.log(`[server]: Server is running at http://localhost:${port}`);
});