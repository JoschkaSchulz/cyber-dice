import HelpService from '../../services/dices/diceHelpService';

const post = (req, res) => {
  res.status(200).json({
    file: req.file,
  });
};

const help = (req, res) => {
  res.status(200).json(HelpService.helpMessage());
};

export { post, help };
