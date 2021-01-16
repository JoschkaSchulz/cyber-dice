import bodyParser from 'body-parser';
import routes from '../api';

export default async ({ app }) => {
  app.use(bodyParser.json());
  app.use('/', routes());
};
