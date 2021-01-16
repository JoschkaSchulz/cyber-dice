import { Router } from 'express';
import dices from './routes/dices';

export default () => {
  const app = Router();
  dices(app);

  return app;
};
