import { Router } from 'express';
import diceUpload from '../middlewares/diceUpload';
import { post, help } from '../controllers/dices';

const route = Router();

export default (app) => {
  app.use('/dices', route);

  route.post('/', diceUpload.single('dice'), post);
  route.get('/help', help);
};
