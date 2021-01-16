import express from 'express';
import loaders from './loaders';

async function startServer() {
  const app = express();

  await loaders({ expressApp: app });

  app.listen(5000, () => {
    console.log('app is listening to port 5000');
  });
}

startServer();
