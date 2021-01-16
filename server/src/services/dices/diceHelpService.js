export default class DiceHelpService {
  static helpMessage() {
    return {
      message: 'Help for dices',
      urls: [
        {
          url: '/dices',
          method: 'POST',
          description: 'Uploads an image of an dice',
        },
      ],
    };
  }
}
