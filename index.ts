
import express, { Express, Request, Response } from "express";
import dotenv from "dotenv";
import path from 'path';
import koffi from 'koffi';

dotenv.config();

const app: Express = express();
const port = process.env.PORT || 3000;

app.get("/", (req: Request, res: Response) => {
    res.send("Express + TypeScript Server");
});

// Load the shared library








const libpath = path.resolve(__dirname, "./example.dll");
const lib = koffi.load(libpath);

// Define the function signatures
const getCounter = lib.func('int get_counter()');
const incrementCounter = lib.func('void increment_counter(int)');
const resetCounter = lib.func('void reset_counter()');

app.get("/koffiadd", (req: Request, res: Response) => {
    incrementCounter(5);
    console.log(getCounter());
    res.send("Express + TypeScript Server");
});
app.get("/koffireset", (req: Request, res: Response) => {
    resetCounter();
    res.send("Express + TypeScript Server");
});

app.get("/createpipeline", (req: Request, res: Response) => {
    const libpath = path.resolve(__dirname, "./mixing.dll");
    const lib = koffi.load(libpath);
    const createPipline = lib.func('int createPipline(string)');
    createPipline("newPipeline");
    res.send("Express + TypeScript Server");
});

app.listen(port, () => {
    console.log(`[server]: Server is running at http://localhost:${port}`);
});




//نکته مهم : وقتی خواستم این برنامه را روی سرور مجازی اوبونتو نصب کنم خطایی برای نصب بسته ای که مربوط به
//اجرای کد های سی بود گرفت که مثل اینکه باید یک بسته ای نصب شود
//میگه یک بسته نصب نیست که کد های زیر را از هوش مصنوعی انجام دادم
// sudo apt-get update
// sudo apt-get install build-essential

// sudo apt-get install python3
// sudo apt-get install pkg-config

//در پوشه برنامه
//npm clean-install

// باز هم خطا گرفت که هز هوش مصنوعی پرسیدم
//در اصل متوجه شدم که بسته های قدیمی تر را باید نصب کنم که قبلی ها را باید پاک کنم و بعد بسته زیر را نصب کنم
//که نصب شد ولی وقتی خواستم برنامه را اجرا کنم همان خطای زیر را می دهد
//npm uninstall ffi-napi ref-napi
//npm install ffi-napi@2.4.6 ref-napi@2.0.3
//! segmentation fault (code dumped);
